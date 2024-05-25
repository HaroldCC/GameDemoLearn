/*************************************************************************
> File Name       : DatabaseWorkerPool.cpp
> Brief           : 数据库工作池
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月18日  11时25分22秒
************************************************************************/
#include "DatabaseWorkerPool.h"
#include "MySqlTypeHack.h"
#include "MySqlConnection.h"
#include "MySqlPreparedStatement.h"
#include "PreparedStatement.h"
#include "Common/Util/Assert.h"

#include "Common/Database/DatabaseImpl/LoginDatabase.h"

namespace Database
{
    template <typename ConnectionType>
    DatabaseWorkerPool<ConnectionType>::DatabaseWorkerPool()
    {
        Assert(mysql_thread_safe(), "数据库不是线程安全的");
    }

    template <typename ConnectionType>
    uint32_t DatabaseWorkerPool<ConnectionType>::Open(const MySqlConnectionInfo &info,
                                                      uint8_t                    syncThreadCount,
                                                      uint8_t                    asyncThreadCount)
    {
        _pConnectionInfo  = std::make_unique<MySqlConnectionInfo>(info);
        _syncThreadCount  = syncThreadCount;
        _asyncThreadCount = asyncThreadCount;

        Log::Info("开始连接数据库：{} 同步方式连接数：{}  异步方式连接数：{}",
                  _pConnectionInfo->database,
                  _syncThreadCount,
                  _asyncThreadCount);

        // _ioContext = std::make_unique<asio::io_context>(_asyncThreadCount);

        uint32_t errcode = OpenConnections(EConnectionTypeIndex_Async, _asyncThreadCount);
        if (0 != errcode)
        {
            return errcode;
        }

        errcode = OpenConnections(EConnectionTypeIndex_Sync, _syncThreadCount);
        if (0 != errcode)
        {
            return errcode;
        }

        for (const auto &pConnection : _typeConnections[EConnectionTypeIndex_Async])
        {
            // std::unique_ptr<asio::io_context>       pIoCtx = std::make_unique<asio::io_context>();
            // std::unique_ptr<asio::io_context::work> pIoCtxWork =
            //     std::make_unique<asio::io_context::work>(*pIoCtx);
            pConnection->StartWorkerThread();
            // _ioContexts.emplace_back(std::move(pIoCtx));
            // _ioCtxWorks.emplace_back(std::move(pIoCtxWork));
        }

        Log::Info("数据库工作池连接成功：{} 当前连接数：{}",
                  _pConnectionInfo->database,
                  (_typeConnections[EConnectionTypeIndex_Sync].size()
                   + _typeConnections[EConnectionTypeIndex_Async].size()));
        return 0;
    }

    template <typename ConnectionType>
    void DatabaseWorkerPool<ConnectionType>::Close()
    {
        Log::Info("关闭数据库工作池：{}", _pConnectionInfo->database);

        // _ioContext->stop();

        _typeConnections[EConnectionTypeIndex_Async].clear();
        // _ioContext->reset();

        _typeConnections[EConnectionTypeIndex_Sync].clear();

        // for (auto &&pWork : _ioCtxWorks)
        // {
        //     pWork->get_io_context().stop();
        //     pWork.reset();
        // }
    }

    template <typename ConnectionType>
    bool DatabaseWorkerPool<ConnectionType>::PrepareStatements()
    {
        for (auto &connections : _typeConnections)
        {
            for (auto &&pConnection : connections)
            {
                pConnection->Lock();
                if (!pConnection->PrepareStatements())
                {
                    pConnection->UnLock();
                    Close();
                    return false;
                }

                pConnection->UnLock();

                const size_t preparedSize = pConnection->_stmts.size();
                if (_preparedStmtParamCount.size() < preparedSize)
                {
                    _preparedStmtParamCount.resize(preparedSize);
                }

                for (size_t i = 0; i < preparedSize; ++i)
                {
                    if (_preparedStmtParamCount[i] > 0)
                    {
                        continue;
                    }

                    if (MySqlPreparedStatement *pStmt = pConnection->_stmts[i].get(); pStmt != nullptr)
                    {
                        const uint32_t paramCount = pStmt->GetParameterCount();
                        Assert(paramCount < std::numeric_limits<uint8_t>::max());
                        _preparedStmtParamCount[i] = static_cast<uint8_t>(paramCount);
                    }
                }
            }
        }

        return true;
    }

    template <typename ConnectionType>
    void DatabaseWorkerPool<ConnectionType>::AsyncExecute(std::string_view sql)
    {
        if (sql.empty())
        {
            return;
        }

        // asio::post(_ioContext->get_executor(), [this, sql] {
        //     for (auto &&pConnection : _typeConnections[EConnectionTypeIndex_Async])
        //     {
        //         if (pConnection->GetWorkerThreadID() == std::this_thread::get_id())
        //         {
        //             return pConnection->Execute(sql);
        //         }
        //     }

        //     Log::Critical("执行sql语句:{}, 错误，找不到对应连接的工作线程", sql);
        //     return false;
        // });
    }

    template <typename ConnectionType>
    void DatabaseWorkerPool<ConnectionType>::AsyncExecute(PreparedStatementBase *pStmt)
    {
        if (nullptr == pStmt)
        {
            return;
        }

        // asio::post(_ioContext->get_executor(), [this, pStmt = std::unique_ptr<PreparedStatementBase>(pStmt)] {
        //     for (auto &&pConnection : _typeConnections[EConnectionTypeIndex_Async])
        //     {
        //         if (pConnection->GetWorkerThreadID() == std::this_thread::get_id())
        //         {
        //             return pConnection->Execute(pStmt.get());
        //         }
        //     }

        //     Log::Critical("执行预处理sql语句:{}, 错误，找不到对应连接的工作线程", pStmt->GetIndex());
        //     return false;
        // });
    }

    template <typename ConnectionType>
    void DatabaseWorkerPool<ConnectionType>::SyncExecute(std::string_view sql)
    {
        if (sql.empty())
        {
            return;
        }

        ConnectionType *pConnection = GetFreeConnectionAndLock();
        pConnection->Execute(sql);
        pConnection->UnLock();
    }

    template <typename ConnectionType>
    void DatabaseWorkerPool<ConnectionType>::SyncExecute(PreparedStatementBase *pStmt)
    {
        if (nullptr == pStmt)
        {
            return;
        }

        ConnectionType *pConnection = GetFreeConnectionAndLock();
        pConnection->Execute(pStmt);
        pConnection->UnLock();
    }

    template <typename ConnectionType>
    QueryCallback DatabaseWorkerPool<ConnectionType>::AsyncQuery(std::string_view sql)
    {
        // QueryResultFuture result =
        //     asio::post(_ioContext->get_executor(), asio::use_future([this, sql]() -> QueryResultSetPtr {
        //                    for (auto &&pConnection : _typeConnections[EConnectionTypeIndex_Async])
        //                    {
        //                        if (pConnection->GetWorkerThreadID() == std::this_thread::get_id())
        //                        {
        //                            return pConnection->Query(sql);
        //                        }
        //                    }

        //                    Log::Critical("执行sql语句:{}, 错误，找不到对应连接的工作线程", sql);
        //                    return nullptr;
        //                }));

        // return QueryCallback(std::move(result));
        return QueryCallback(QueryResultFuture());
    }

    template <typename ConnectionType>
    QueryCallback DatabaseWorkerPool<ConnectionType>::AsyncQuery(PreparedStatementBase *pStmt)
    {
        // PreparedQueryResultFuture result = asio::post(
        //     _ioContext->get_executor(),
        //     asio::use_future(
        //         [this, pStmt = std::unique_ptr<PreparedStatementBase>(pStmt)]() -> PreparedQueryResultSetPtr {
        //             for (auto &&pConnection : _typeConnections[EConnectionTypeIndex_Async])
        //             {
        //                 if (pConnection->GetWorkerThreadID() == std::this_thread::get_id())
        //                 {
        //                     return pConnection->Query(pStmt.get());
        //                 }
        //             }

        //             Log::Critical("执行预处理sql语句:{}, 错误，找不到对应连接的工作线程", pStmt->GetIndex());
        //             return nullptr;
        //         }));

        // return QueryCallback(std::move(result));
        return QueryCallback(QueryResultFuture());
    }

    template <typename ConnectionType>
    void DatabaseWorkerPool<ConnectionType>::CoQuery(std::string_view                         sql,
                                                     std::function<void(QueryResultSetPtr)> &&f)
    {
        auto pConnection = GetFreeConnectionAndLock();
        pConnection->UnLock();
        asio::co_spawn(pConnection->GetIoContext(),
                       pConnection->CoQuery(sql),
                       [](std::exception_ptr ex, QueryResultSetPtr pResult) {
                           //    f(pResult);
                           Log::Debug("result");
                       });
    }

    template <typename ConnectionType>
    QueryResultSetPtr DatabaseWorkerPool<ConnectionType>::SyncQuery(std::string_view sql)
    {
        if (sql.empty())
        {
            return nullptr;
        }

        ConnectionType   *pConnection = GetFreeConnectionAndLock();
        QueryResultSetPtr pResult     = pConnection->Query(sql);
        pConnection->UnLock();

        return pResult;
    }

    template <typename ConnectionType>
    PreparedQueryResultSetPtr DatabaseWorkerPool<ConnectionType>::SyncQuery(PreparedStatementBase *pStmt)
    {
        if (nullptr == pStmt)
        {
            return nullptr;
        }

        ConnectionType           *pConnection = GetFreeConnectionAndLock();
        PreparedQueryResultSetPtr pResult     = pConnection->Query(pStmt);
        pConnection->UnLock();

        return pResult;
    }

    template <typename ConnectionType>
    uint32_t DatabaseWorkerPool<ConnectionType>::OpenConnections(EConnectionTypeIndex type,
                                                                 uint8_t              openConnectionCount)
    {
        for (uint8_t i = 0; i < openConnectionCount; ++i)
        {
            constexpr std::array<MySqlConnectionType, EConnectionTypeIndex_Max> connectionTypes {
                MySqlConnectionType::Async,
                MySqlConnectionType::Sync};

            std::unique_ptr<ConnectionType> pConnection =
                std::make_unique<ConnectionType>(*_pConnectionInfo, connectionTypes[type]);

            Assert(nullptr != pConnection);

            if (uint32_t errcode = pConnection->Open(); errcode != 0)
            {
                _typeConnections[type].clear();
                return errcode;
            }

            _typeConnections[type].emplace_back(std::move(pConnection));
        }

        return 0;
    }

    template <typename ConnectionType>
    ConnectionType *DatabaseWorkerPool<ConnectionType>::GetFreeConnectionAndLock()
    {
        const auto      connectionCount = _typeConnections[EConnectionTypeIndex_Sync].size();
        ConnectionType *pConnection     = nullptr;
        uint8_t         index           = 0;
        while (nullptr == pConnection)
        {
            pConnection = _typeConnections[EConnectionTypeIndex_Sync][index++ % connectionCount].get();
            if (pConnection->TryLock())
            {
                break;
            }
        }

        return pConnection;
    }

    template <typename ConnectionType>
    PreparedStatementBase *DatabaseWorkerPool<ConnectionType>::GetPrepareStatement(uint32_t stmtID) const
    {
        return new PreparedStatementBase(stmtID, _preparedStmtParamCount[stmtID]);
    }

    template class DatabaseWorkerPool<LoginDatabaseConnection>;
} // namespace Database