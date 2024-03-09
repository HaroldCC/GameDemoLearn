/*************************************************************************
> File Name       : LoginDatabase.h
> Brief           : 登录数据库
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月22日  15时57分20秒
************************************************************************/
#pragma once

#include "Common/Database/DatabaseWorkerPool.h"
#include "Common/Database/MySqlConnection.h"
#include "Common/Database/PreparedStatement.h"

#include <string_view>
#include <vector>
#include <map>

namespace Database
{

    enum LoginDatabaseSqlID : uint32_t
    {
        LOGIN_SEL_ACCOUNT_BY_EMAIL,
        LOGIN_SEL_ACCOUNT_BY_EMAIL2,

        LOGIN_DATABASE_SQL_ID_MAX,
    };

    const std::map<LoginDatabaseSqlID, SqlStmtData> g_LoginDatabaseStmts =
        //
        {
            {
                LoginDatabaseSqlID::LOGIN_SEL_ACCOUNT_BY_EMAIL,
                {"select id, name, email, age, intro from account where email = ?",
                 {SqlArgType::String},
                 MySqlConnectionType::Async_Sync}
                //
            },

            {
                LoginDatabaseSqlID::LOGIN_SEL_ACCOUNT_BY_EMAIL2,
                {"select id, name, email, age, intro from account where email = ?",
                 {SqlArgType::String},
                 MySqlConnectionType::Async}
                //
            },
            //
    };

    class LoginDatabaseConnection : public IMySqlConnection
    {
    public:
        using IMySqlConnection::IMySqlConnection;

        void DoPrepareStatements() override;
    };

    extern DatabaseWorkerPool<LoginDatabaseConnection> g_LoginDatabase;

} // namespace Database