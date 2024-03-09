/*************************************************************************
> File Name       : MySqlPreparedStatement.h
> Brief           : MySql预处理语句
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月11日  17时28分07秒
************************************************************************/
#pragma once

#include "DatabaseEnv.h"

#include <string>
#include <vector>

namespace Database
{
    class MySqlPreparedStatement
    {
        friend class IMySqlConnection;
        friend class PreparedStatementBase;

    public:
        MySqlPreparedStatement(const MySqlPreparedStatement &)            = delete;
        MySqlPreparedStatement &operator=(const MySqlPreparedStatement &) = delete;
        MySqlPreparedStatement(MySqlPreparedStatement &&)                 = delete;
        MySqlPreparedStatement &operator=(MySqlPreparedStatement &&)      = delete;

        MySqlPreparedStatement(MySqlStmt *pStmt, std::string_view sql);
        ~MySqlPreparedStatement();

        void BindParameters(PreparedStatementBase *pStmt);

        [[nodiscard]] uint32_t GetParameterCount() const
        {
            return _paramCount;
        }

    private:
        template <typename T>
        void SetParameter(uint8_t index, T &&value);

        void SetParameter(uint8_t index, std::nullptr_t);
        void SetParameter(uint8_t index, bool value);
        void SetParameter(uint8_t index, const std::string &str);
        void SetParameter(uint8_t index, const std::vector<uint8_t> &value);

        void AssertValidIndex(uint8_t index);

        [[nodiscard]] MySqlStmt *GetMySqlStmt() const
        {
            return _pMySqlStmt;
        }

        [[nodiscard]] MySqlBind *GetMySqlBind() const
        {
            return _pBind;
        }

        void ClearParameters();

        [[nodiscard]] std::string GetSqlString() const;

    private:
        PreparedStatementBase *_pStmt {nullptr};
        MySqlStmt             *_pMySqlStmt {nullptr};
        uint32_t               _paramCount {0};
        std::vector<bool>      _paramAssignFlag;
        MySqlBind             *_pBind {nullptr};
        std::string_view       _sqlString;
    };
} // namespace Database