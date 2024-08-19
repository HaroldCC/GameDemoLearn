/*************************************************************************
> File Name       : MySqlConnection.h
> Brief           : 数据库连接
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月11日  11时14分33秒
************************************************************************/
#pragma once

#include "DatabaseEnv.h"
#include "asio.hpp"
#include "MySqlPreparedStatement.h"

#include <cstdint>
#include <string_view>
#include <vector>
#include <mutex>

namespace Database
{
    class PreparedStatementBase;

    struct MySqlConnectionInfo
    {
        std::string_view user;
        std::string_view password;
        std::string_view database;
        std::string_view host;
        std::string_view port;
    };

    class IMySqlConnection
    {
        template <typename ConnectionType>
        friend class DatabaseWorkerPool;

    public:
        IMySqlConnection(const IMySqlConnection &)            = delete;
        IMySqlConnection(IMySqlConnection &&)                 = delete;
        IMySqlConnection &operator=(const IMySqlConnection &) = delete;
        IMySqlConnection &operator=(IMySqlConnection &&)      = delete;

        explicit IMySqlConnection(MySqlConnectionInfo &info, MySqlConnectionType connType);
        virtual ~IMySqlConnection();

        uint32_t Open();
        void     Close();

        bool PrepareStatements();

        bool Execute(std::string_view sql);
        bool Execute(PreparedStatementBase *pStmt);

        void AsyncExecute(std::string_view sql);
        void AsyncExecute(PreparedStatementBase *pStmt);

        QueryResultSetPtr         Query(std::string_view sql);
        PreparedQueryResultSetPtr Query(PreparedStatementBase *pStmt);

        QueryResultFuture         AsyncQuery(std::string_view sql);
        PreparedQueryResultFuture AsyncQuery(PreparedStatementBase *pStmt);

        void BeginTransaction();
        void CommitTransaction();
        void RollbackTransaction();

        uint32_t GetLastError();

        void Ping() const;

        void StartWorkerThread();

        std::thread::id GetWorkerThreadID() const
        {
            return _pWorkerThread->get_id();
        }

        std::size_t GetAsyncTaskCount() const
        {
            return _asyncTaskCount;
        }

    protected:
        virtual void DoPrepareStatements() = 0;
        void         PrepareStatement(uint32_t index, std::string_view sql, MySqlConnectionType connType);
        MySqlPreparedStatement *GetPrepareStatement(uint32_t index);

        bool HandleMySqlErrcode(uint32_t errcode, uint8_t tryReconnectTimes = 5);

        bool Query(std::string_view sql,
                   MySqlResult    *&pResult,
                   MySqlField     *&pFields,
                   uint64_t        &rowCount,
                   uint32_t        &fieldCount);
        bool Query(PreparedStatementBase   *pStmt,
                   MySqlPreparedStatement *&pMySqlProxyStmt,
                   MySqlResult            *&pResult,
                   uint64_t                &rowCount,
                   uint32_t                &fieldCount);

        void Lock()
        {
            _mutex.lock();
        }

        bool TryLock()
        {
            return _mutex.try_lock();
        }

        void UnLock()
        {
            _mutex.unlock();
        }

    protected:
        using PreparedStatementContainer = std::vector<std::unique_ptr<MySqlPreparedStatement>>;
        PreparedStatementContainer _stmts;
        bool                       _bReconnecting {false};
        bool                       _bPrepareError {false};

        bool Update();

    private:
        std::unique_ptr<std::thread> _pWorkerThread;
        MySqlHandle                 *_pMysqlHandle {nullptr};
        MySqlConnectionInfo         &_connectInfo;
        MySqlConnectionType          _mysqlConnType;
        asio::io_context             _ioCtx;
        asio::any_io_executor        _ioWork;
        std::size_t                  _asyncTaskCount = 0;
        std::mutex                   _mutex;
    };
} // namespace Database