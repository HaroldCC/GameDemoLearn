/*************************************************************************
> File Name       : DatabaseWorkerPool.h
> Brief           : 数据库工作池
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月17日  17时15分30秒
************************************************************************/
#pragma once

#include "asio.hpp"

#include "DatabaseEnv.h"
#include "QueryCallback.h"

#include <array>
#include <vector>
#include <memory>

namespace Database
{
    class IMySqlConnection;
    struct MySqlConnectionInfo;
    class PreparedStatementBase;
    class QueryCallback;

    template <typename ConnectionType>
    class DatabaseWorkerPool final
    {
    private:
        enum EConnectionTypeIndex
        {
            EConnectionTypeIndex_Async,
            EConnectionTypeIndex_Sync,
            EConnectionTypeIndex_Max,
        };

    public:
        DatabaseWorkerPool(const DatabaseWorkerPool &)            = delete;
        DatabaseWorkerPool(DatabaseWorkerPool &&)                 = delete;
        DatabaseWorkerPool &operator=(const DatabaseWorkerPool &) = delete;
        DatabaseWorkerPool &operator=(DatabaseWorkerPool &&)      = delete;

        DatabaseWorkerPool();
        ~DatabaseWorkerPool() = default;

        uint32_t Open(const MySqlConnectionInfo &info, uint8_t syncThreadCount, uint8_t asyncThreadCount);
        void     Close();

        bool PrepareStatements();

        void AsyncExecute(std::string_view sql);
        void AsyncExecute(PreparedStatementBase *pStmt);

        void SyncExecute(std::string_view sql);
        void SyncExecute(PreparedStatementBase *pStmt);

        QueryCallback AsyncQuery(std::string_view sql);
        QueryCallback AsyncQuery(PreparedStatementBase *pStmt);

        QueryResultSetPtr         SyncQuery(std::string_view sql);
        PreparedQueryResultSetPtr SyncQuery(PreparedStatementBase *pStmt);

        PreparedStatementBase *GetPrepareStatement(uint32_t stmtID) const;

    private:
        uint32_t OpenConnections(EConnectionTypeIndex type, uint8_t openConnectionCount);

        std::shared_ptr<ConnectionType> GetFreeConnectionAndLock();

        std::shared_ptr<ConnectionType> GetFreeAsyncConnection();

    private:
        std::array<std::vector<std::shared_ptr<ConnectionType>>, EConnectionTypeIndex_Max> _typeConnections;
        std::atomic<size_t>                                                                _queueSize;
        std::unique_ptr<MySqlConnectionInfo>                                               _pConnectionInfo;
        std::vector<uint8_t> _preparedStmtParamCount;
        uint8_t              _asyncThreadCount {0};
        uint8_t              _syncThreadCount {0};
    };
} // namespace Database