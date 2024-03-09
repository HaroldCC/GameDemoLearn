/*************************************************************************
> File Name       : LoginDatabase.cpp
> Brief           : 登录数据库
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月22日  16时53分54秒
************************************************************************/
#include "LoginDatabase.h"

#include "Common/Util/Log.h"
#include "Common/Util//Util.h"

namespace Database
{
    DatabaseWorkerPool<LoginDatabaseConnection> g_LoginDatabase;

    void LoginDatabaseConnection::DoPrepareStatements()
    {
        if (!_bReconnecting)
        {
            _stmts.resize(Util::ToUnderlying(LoginDatabaseSqlID::LOGIN_DATABASE_SQL_ID_MAX));
        }

        for (const auto &sqlStmt : g_LoginDatabaseStmts)
        {
            PrepareStatement(Util::ToUnderlying(sqlStmt.first),
                             sqlStmt.second.sql,
                             sqlStmt.second.connectionType);
        }
    }
} // namespace Database