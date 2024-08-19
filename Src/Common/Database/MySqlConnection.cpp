/*************************************************************************
> File Name       : MySqlConnection.cpp
> Brief           : 数据库连接
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月11日  15时37分09秒
************************************************************************/
#include "MySqlConnection.h"
#include "PreparedStatement.h"
#include "MySqlPreparedStatement.h"
#include "QueryResult.h"

#include "MySqlTypeHack.h"
#include "mysqld_error.h"

#include "Common/Util/Log.h"
#include "Common/Util/Util.h"
#include "Common/Util/Performance.h"

namespace Database
{

    IMySqlConnection::IMySqlConnection(MySqlConnectionInfo &info, MySqlConnectionType connType)
        : _connectInfo(info)
        , _mysqlConnType(connType)
        , _ioCtx(1)
    {
        Log::Debug("create MysqlConnection");
    }

    bool IMySqlConnection::Update()
    {
        return true;
    }

    IMySqlConnection::~IMySqlConnection()
    {
        Close();
    }

    uint32_t IMySqlConnection::Open()
    {
        _pMysqlHandle = mysql_init(nullptr);
        if (nullptr == _pMysqlHandle)
        {
            Log::Error("初始化数据库：{} 失败", _connectInfo.database);
            return CR_UNKNOWN_ERROR;
        }

        mysql_options(_pMysqlHandle, MYSQL_SET_CHARSET_NAME, "utf8mb4");

        uint32_t port = Util::StringTo<uint32_t>(_connectInfo.port).value_or(3306);
        _pMysqlHandle = mysql_real_connect(_pMysqlHandle,
                                           _connectInfo.host.data(),
                                           _connectInfo.user.data(),
                                           _connectInfo.password.data(),
                                           _connectInfo.database.data(),
                                           port,
                                           nullptr,
                                           0);
        if (nullptr == _pMysqlHandle)
        {
            uint32_t errcode = mysql_errno(_pMysqlHandle);
            Log::Error("数据库连接失败：{}:{} [host:{}, user:{}, password:{}, database:{}, port:{}]",
                       errcode,
                       mysql_errno(_pMysqlHandle),
                       _connectInfo.host,
                       _connectInfo.user,
                       _connectInfo.password,
                       _connectInfo.database,
                       port);
            mysql_close(_pMysqlHandle);
            return errcode;
        }

        if (!_bReconnecting)
        {
            Log::Info("MySql client library:{}", mysql_get_client_info());
            Log::Info("MySql server version:{}", mysql_get_server_info(_pMysqlHandle));
        }

        mysql_autocommit(_pMysqlHandle, true);
        return 0;
    }

    void IMySqlConnection::Close()
    {
        Log::Debug("mysqlconnection close");
        // _workGrard.reset();

        if (_pWorkerThread->joinable())
        {
            _pWorkerThread->join();
        }

        _stmts.clear();

        if (nullptr != _pMysqlHandle)
        {
            mysql_close(_pMysqlHandle);
            _pMysqlHandle = nullptr;
        }
    }

    bool IMySqlConnection::PrepareStatements()
    {
        DoPrepareStatements();
        return !_bPrepareError;
    }

    bool IMySqlConnection::Execute(std::string_view sql)
    {
        if (nullptr == _pMysqlHandle)
        {
            return false;
        }

        {
            PERFORMANCE_SCOPE("执行Sql语句：{}", sql);

            if (0 != mysql_query(_pMysqlHandle, sql.data()))
            {
                uint32_t errcode = mysql_errno(_pMysqlHandle);
                Log::Info("执行sql脚本：{}", sql);
                Log::Error("执行sql脚本出错:[{}]:{}", errcode, mysql_errno(_pMysqlHandle));

                if (HandleMySqlErrcode(errcode))
                {
                    return Execute(sql);
                }

                return false;
            }
        }

        return true;
    }

    bool IMySqlConnection::Execute(PreparedStatementBase *pStmt)
    {
        if (nullptr == _pMysqlHandle)
        {
            return false;
        }

        uint32_t                index         = pStmt->GetIndex();
        MySqlPreparedStatement *pPreparedStmt = GetPrepareStatement(index);
        Assert(nullptr != pPreparedStmt);

        pPreparedStmt->BindParameters(pStmt);

        MySqlStmt *pMySqlStmt = pPreparedStmt->GetMySqlStmt();
        MySqlBind *pMySqlBind = pPreparedStmt->GetMySqlBind();

        {
            PERFORMANCE_SCOPE("执行sql语句：{}", pPreparedStmt->GetSqlString());

            if (mysql_stmt_bind_param(pMySqlStmt, pMySqlBind))
            {
                uint32_t errcode = mysql_errno(_pMysqlHandle);
                Log::Error("执行sql:{} 绑定参数错误：{}:{}",
                           pPreparedStmt->GetSqlString(),
                           errcode,
                           mysql_stmt_error(pMySqlStmt));

                if (HandleMySqlErrcode(errcode))
                {
                    return Execute(pStmt);
                }

                pPreparedStmt->ClearParameters();
                return false;
            }

            if (0 != mysql_stmt_execute(pMySqlStmt))
            {
                uint32_t errcode = mysql_errno(_pMysqlHandle);
                Log::Error("执行sql语句：{} 出错：{}:{}",
                           pPreparedStmt->GetSqlString(),
                           errcode,
                           mysql_stmt_error(pMySqlStmt));

                if (HandleMySqlErrcode(errcode))
                {
                    return Execute(pStmt);
                }

                pPreparedStmt->ClearParameters();
                return false;
            }
        }

        pPreparedStmt->ClearParameters();
        return true;
    }

    void IMySqlConnection::AsyncExecute(std::string_view sql)
    {
        if (sql.empty())
        {
            return;
        }

        asio::post(_ioWork, [this, strSql = std::string(sql)]() {
            if (!Execute(strSql))
            {
                // do nothing
            }
        });
    }

    void IMySqlConnection::AsyncExecute(PreparedStatementBase *pStmt)
    {
        if (nullptr == pStmt)
        {
            return;
        }

        asio::post(_ioWork, [this, pStmt = std::unique_ptr<PreparedStatementBase>(pStmt)]() {
            if (!Execute(pStmt.get()))
            {
                // do nothing
            }
        });
    }

    QueryResultSetPtr IMySqlConnection::Query(std::string_view sql)
    {
        if (sql.empty())
        {
            return nullptr;
        }

        MySqlResult       *pResult    = nullptr;
        MySqlField        *pFields    = nullptr;
        uint64_t           rowCount   = 0;
        uint32_t           fieldCount = 0;
        std::ostringstream ss;
        ss << std::this_thread::get_id();
        Log::Debug("**********Query thread id:{}", ss.str());
        if (!Query(sql, pResult, pFields, rowCount, fieldCount))
        {
            return nullptr;
        }

        return MakeQueryResultSetPtr(pResult, pFields, rowCount, fieldCount);
    }

    PreparedQueryResultSetPtr IMySqlConnection::Query(PreparedStatementBase *pStmt)
    {
        MySqlPreparedStatement *pPreparedStmt = nullptr;
        MySqlResult            *pResult       = nullptr;
        uint64_t                rowCount      = 0;
        uint32_t                fieldCount    = 0;
        std::ostringstream      ss;
        ss << std::this_thread::get_id();
        Log::Debug("**********Query thread id:{}", ss.str());
        if (!Query(pStmt, pPreparedStmt, pResult, rowCount, fieldCount))
        {
            return nullptr;
        }

        return MakePreparedQueryResultSetPtr(pPreparedStmt->GetMySqlStmt(), pResult, rowCount, fieldCount);
    }

    void IMySqlConnection::BeginTransaction()
    {
        Execute("START TRANSACTION");
    }

    void IMySqlConnection::CommitTransaction()
    {
        Execute("COMMIT");
    }

    void IMySqlConnection::RollbackTransaction()
    {
        Execute("ROLLBACK");
    }

    uint32_t IMySqlConnection::GetLastError()
    {
        return mysql_errno(_pMysqlHandle);
    }

    void IMySqlConnection::Ping() const
    {
        mysql_ping(_pMysqlHandle);
    }

    void IMySqlConnection::StartWorkerThread()
    {
        _ioWork        = asio::require(_ioCtx.get_executor(), asio::execution::outstanding_work.tracked);
        _pWorkerThread = std::make_unique<std::thread>([this] {
            try
            {
                Log::Debug("mysql running.....");
                std::error_code errcode;
                _ioCtx.run();
                if (errcode)
                {
                    Log::Error("sql connection running failed:{}", errcode.message());
                }

                Log::Debug("mysql stoping.....");
            }
            catch (std::exception &e)
            {
                Log::Error("数据库线程发生异常:{}", e.what());
            }
        });
    }

    void
    IMySqlConnection::PrepareStatement(uint32_t index, std::string_view sql, MySqlConnectionType connType)
    {
        if (!(Util::ToUnderlying(_mysqlConnType) & Util::ToUnderlying(connType)))
        {
            _stmts[index].reset();
            return;
        }

        MySqlStmt *pStmt = mysql_stmt_init(_pMysqlHandle);
        if (nullptr == pStmt)
        {
            Log::Error("初始化预处理语句：{}：{} 错误：{}", index, sql, mysql_error(_pMysqlHandle));
            _bPrepareError = true;
        }
        else
        {
            if (0 != mysql_stmt_prepare(pStmt, sql.data(), static_cast<uint32_t>(sql.size())))
            {
                Log::Error("处理预处理语句:{}:{} 错误：{}", index, sql, mysql_stmt_error(pStmt));
                mysql_stmt_close(pStmt);
                _bPrepareError = true;
            }
            else
            {
                _stmts[index] = std::make_unique<MySqlPreparedStatement>(pStmt, sql);
            }
        }
    }

    MySqlPreparedStatement *IMySqlConnection::GetPrepareStatement(uint32_t index)
    {
        Assert(
            index < _stmts.size(),
            std::format("尝试获取非法的预处理语句，索引：{} (最大索引：{}) 数据库：{}, 连接方式：{}",
                        index,
                        _stmts.size(),
                        _connectInfo.database,
                        ((uint8_t)_mysqlConnType & (uint8_t)MySqlConnectionType::Async) ? "异步" : "同步"));
        MySqlPreparedStatement *pMySqlPreparedStmt = _stmts[index].get();
        if (nullptr == pMySqlPreparedStmt)
        {
            Log::Error("获取数据库：{} 预处理语句索引 {} 失败，连接方式：{}",
                       _connectInfo.database,
                       index,
                       (Util::ToUnderlying(_mysqlConnType) & Util::ToUnderlying(MySqlConnectionType::Async))
                           ? "异步"
                           : "同步");
        }

        return pMySqlPreparedStmt;
    }

    bool IMySqlConnection::HandleMySqlErrcode(uint32_t errcode, uint8_t tryReconnectTimes /*= 5*/)
    {
        using namespace std::chrono_literals;
        switch (errcode)
        {
            case CR_SERVER_GONE_ERROR:
            case CR_SERVER_LOST:
            case CR_SERVER_LOST_EXTENDED:
            {
                if (nullptr != _pMysqlHandle)
                {
                    Log::Error("Mysql server 连接丢失");
                    mysql_close(_pMysqlHandle);
                    _pMysqlHandle = nullptr;
                }
            }
            case CR_CONN_HOST_ERROR:
            {
                Log::Info("Reconnecting Mysql server......");
                _bReconnecting         = true;
                const uint32_t errcode = Open();
                if (0 != errcode)
                {
                    if (!this->PrepareStatements())
                    {
                        Log::Critical("处理预处理sql语句失败，数据库连接即将断开!!!");
                        std::this_thread::sleep_for(10s);
                        std::abort();
                    }

                    Log::Info("重连数据库成功：{} @{}:{} {}",
                              _connectInfo.database,
                              _connectInfo.host,
                              _connectInfo.port,
                              ((uint8_t)_mysqlConnType & (uint8_t)MySqlConnectionType::Async) ? "异步方式"
                                                                                              : "同步方式");
                    _bReconnecting = false;
                    return true;
                }

                if ((--tryReconnectTimes) == 0)
                {
                    Log::Critical("多次尝试重连数据库失败，服务中断！！！");
                    std::this_thread::sleep_for(10s);
                    std::abort();
                }
                else
                {
                    std::this_thread::sleep_for(3s);
                    return HandleMySqlErrcode(_bPrepareError, tryReconnectTimes);
                }
            }
            case ER_LOCK_DEADLOCK:
            // Implemented in TransactionTask::Execute and DatabaseWorkerPool<T>::DirectCommitTransaction
            // Query related errors - skip query
            case ER_WRONG_VALUE_COUNT:
            case ER_DUP_ENTRY:
                return false;

            // Outdated table or database structure - terminate core
            case ER_BAD_FIELD_ERROR:
            case ER_NO_SUCH_TABLE:
                Log::Error("数据库未找到对应的表");
                std::this_thread::sleep_for(std::chrono::seconds(10));
                std::abort();
                return false;
            case ER_PARSE_ERROR:
                Log::Error("数据库脚本出错，请检查语法");
                std::this_thread::sleep_for(std::chrono::seconds(10));
                std::abort();
                return false;
            default:
            {
                Log::Error("未处理的Mysql 错误，错误码：{}", errcode);
                return false;
            }
        }

        return false;
    }

    bool IMySqlConnection::Query(std::string_view sql,
                                 MySqlResult    *&pResult,
                                 MySqlField     *&pFields,
                                 uint64_t        &rowCount,
                                 uint32_t        &fieldCount)
    {
        if (nullptr == _pMysqlHandle)
        {
            return false;
        }

        {
            PERFORMANCE_SCOPE("执行sql语句：{}", sql);

            if (0 != mysql_query(_pMysqlHandle, sql.data()))
            {
                uint32_t errcode = mysql_errno(_pMysqlHandle);
                Log::Error("执行sql语句：{} 出错：{}：{}", sql, errcode, mysql_error(_pMysqlHandle));
                if (HandleMySqlErrcode(errcode))
                {
                    return Query(sql, pResult, pFields, rowCount, fieldCount);
                }

                return false;
            }
        }

        pResult    = mysql_store_result(_pMysqlHandle);
        rowCount   = mysql_affected_rows(_pMysqlHandle);
        fieldCount = mysql_field_count(_pMysqlHandle);

        if (nullptr == pResult)
        {
            return false;
        }

        if (0 == rowCount)
        {
            mysql_free_result(pResult);
            return false;
        }

        pFields = mysql_fetch_fields(pResult);

        return true;
    }

    bool IMySqlConnection::Query(PreparedStatementBase   *pStmt,
                                 MySqlPreparedStatement *&pMySqlProxyStmt,
                                 MySqlResult            *&pResult,
                                 uint64_t                &rowCount,
                                 uint32_t                &fieldCount)
    {
        if (nullptr == _pMysqlHandle)
        {
            return false;
        }

        uint32_t index  = pStmt->GetIndex();
        pMySqlProxyStmt = GetPrepareStatement(index);
        Assert(nullptr != pMySqlProxyStmt);

        pMySqlProxyStmt->BindParameters(pStmt);

        MySqlStmt *pMySqlStmt = pMySqlProxyStmt->GetMySqlStmt();
        MySqlBind *pMySqlBind = pMySqlProxyStmt->GetMySqlBind();

        {
            PERFORMANCE_SCOPE("执行sql语句：{}", pMySqlProxyStmt->GetSqlString());

            if (mysql_stmt_bind_param(pMySqlStmt, pMySqlBind))
            {
                uint32_t errcode = mysql_errno(_pMysqlHandle);
                Log::Error("预处理语句：{} 绑定参数错误：{}：{}",
                           pMySqlProxyStmt->GetSqlString(),
                           errcode,
                           mysql_stmt_error(pMySqlStmt));

                if (HandleMySqlErrcode(errcode))
                {
                    return Query(pStmt, pMySqlProxyStmt, pResult, rowCount, fieldCount);
                }

                pMySqlProxyStmt->ClearParameters();
                return false;
            }

            if (0 != mysql_stmt_execute(pMySqlStmt))
            {
                uint32_t errcode = mysql_errno(_pMysqlHandle);
                Log::Error("执行预处理语句：{} 错误：{}：{}",
                           pMySqlProxyStmt->GetSqlString(),
                           errcode,
                           mysql_stmt_error(pMySqlStmt));

                if (HandleMySqlErrcode(errcode))
                {
                    return Query(pStmt, pMySqlProxyStmt, pResult, rowCount, fieldCount);
                }

                pMySqlProxyStmt->ClearParameters();
                return false;
            }
        }

        pMySqlProxyStmt->ClearParameters();

        pResult    = mysql_stmt_result_metadata(pMySqlStmt);
        rowCount   = mysql_stmt_num_rows(pMySqlStmt);
        fieldCount = mysql_stmt_field_count(pMySqlStmt);

        return true;
    }

    QueryResultFuture IMySqlConnection::AsyncQuery(std::string_view sql)
    {
        ++_asyncTaskCount;
        return asio::post(_ioCtx.get_executor(), asio::use_future([this, sql]() -> QueryResultSetPtr {
                              auto result = Query(sql);
                              --_asyncTaskCount;
                              return result;
                          }));
    }

    PreparedQueryResultFuture IMySqlConnection::AsyncQuery(PreparedStatementBase *pStmt)
    {
        std::ostringstream ss;
        ss << std::this_thread::get_id();
        Log::Warn("IMySqlConnection::AsyncQuery:{}", ss.str());
        ++_asyncTaskCount;

        return asio::post(
            _ioWork,
            asio::use_future(
                [this, p = std::shared_ptr<PreparedStatementBase>(pStmt)]() -> PreparedQueryResultSetPtr {
                    std::ostringstream ss;
                    ss << std::this_thread::get_id();
                    Log::Warn("IMySqlConnection::AsyncQuery running:{}", ss.str());
                    auto result = Query(p.get());
                    --_asyncTaskCount;
                    return result;
                }));
    }
} // namespace Database