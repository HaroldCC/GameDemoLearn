/*************************************************************************
> File Name       : DatabaseEnv.h
> Brief           : 数据库声明
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月11日  14时39分39秒
************************************************************************/
#pragma once

#include <memory>
#include <future>

using MySqlHandle = struct MYSQL;
using MySqlResult = struct MYSQL_RES;
using MySqlField  = struct MYSQL_FIELD;
using MySqlBind   = struct MYSQL_BIND;
using MySqlStmt   = struct MYSQL_STMT;

namespace Database
{
    enum class SqlArgType : uint32_t
    {
        Bool,
        Uint8,
        Uint16,
        Uint32,
        Uint64,
        Int8,
        Int16,
        Int32,
        Int64,
        Float,
        Double,
        String,
        Binary,
        Null,
    };

    enum class MySqlConnectionType : uint8_t
    {
        Async      = 0x1,
        Sync       = 0x2,
        Async_Sync = 0x1 | 0x2,
    };

    struct SqlStmtData
    {
        std::string_view        sql;
        std::vector<SqlArgType> argtypes;
        MySqlConnectionType     connectionType;

        constexpr SqlStmtData(std::string_view                  sql,
                              std::initializer_list<SqlArgType> types,
                              MySqlConnectionType               connType)
            : sql(sql)
            , argtypes(types)
            , connectionType(connType)
        {
        }
    };

    class QueryResultSet;
    using QueryResultSetPtr = std::shared_ptr<QueryResultSet>;

    template <typename... Args>
    inline QueryResultSetPtr MakeQueryResultSetPtr(Args &&...args)
    {
        return std::make_shared<QueryResultSet>(std::forward<Args>(args)...);
    }

    class PreparedQueryResultSet;
    using PreparedQueryResultSetPtr = std::shared_ptr<PreparedQueryResultSet>;

    template <typename... Args>
    inline PreparedQueryResultSetPtr MakePreparedQueryResultSetPtr(Args &&...args)
    {
        return std::make_shared<PreparedQueryResultSet>(std::forward<Args>(args)...);
    }

    using QueryResultFuture  = std::future<QueryResultSetPtr>;
    using QueryResultPromise = std::promise<QueryResultSetPtr>;

    using PreparedQueryResultFuture  = std::future<PreparedQueryResultSetPtr>;
    using PreparedQueryResultPromise = std::future<PreparedQueryResultSetPtr>;

} // namespace Database