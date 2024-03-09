/*************************************************************************
> File Name       : MySqlPreparedStatement.cpp
> Brief           : MySql预处理语句
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月11日  17时35分30秒
************************************************************************/
#include "MySqlPreparedStatement.h"
#include "PreparedStatement.h"
#include "MySqlTypeHack.h"
#include "Common/Util/Assert.h"
#include "Common/Util/Log.h"

namespace Database
{
    MySqlPreparedStatement::MySqlPreparedStatement(MySqlStmt *pStmt, std::string_view sql)
        : _pMySqlStmt(pStmt)
        , _paramCount(mysql_stmt_param_count(_pMySqlStmt))
        , _pBind(new MySqlBind[_paramCount])
        , _sqlString(sql)
    {
        std::memset(_pBind, 0, sizeof(MySqlBind) * _paramCount);

        _paramAssignFlag.assign(_paramCount, false);

        MySqlBool boolTmp = true;
        mysql_stmt_attr_set(_pMySqlStmt, STMT_ATTR_UPDATE_MAX_LENGTH, &boolTmp);
    }

    MySqlPreparedStatement::~MySqlPreparedStatement()
    {
        ClearParameters();

        if (_pMySqlStmt->bind_result_done)
        {
            delete[] _pMySqlStmt->bind->length;
            delete _pMySqlStmt->bind->is_null;
        }

        mysql_stmt_close(_pMySqlStmt);
        delete[] _pBind;
    }

    void MySqlPreparedStatement::BindParameters(PreparedStatementBase *pStmt)
    {
        _pStmt = pStmt;

        uint8_t pos = 0;

        for (const PreparedStatementData &data : _pStmt->GetParameters())
        {
            std::visit(
                [&](auto &&param) {
                    SetParameter(pos, param);
                },
                data.data);
            ++pos;
        }

#ifdef DEBUG
        if (pos < _paramCount)
        {
            Log::Warn("预处理语句id：{} 未成功绑定所有参数", _pStmt->GetIndex());
        }
#endif
    }

    template <typename T>
    void MySqlPreparedStatement::SetParameter(uint8_t index, T &&value)
    {
        AssertValidIndex(index);
        _paramAssignFlag[index] = true;
        MySqlBind *pParam       = &_pBind[index];
        uint32_t   length       = sizeof(T);
        pParam->buffer_type     = MySqlType<std::decay_t<T>>::value;
        delete[] static_cast<char *>(pParam->buffer);
        pParam->buffer        = new char[length];
        pParam->buffer_length = 0;
        pParam->is_null_value = false;
        pParam->length        = nullptr;
        pParam->is_unsigned   = std::is_signed_v<T>;

        memcpy(pParam->buffer, &value, length);
    }

    void MySqlPreparedStatement::SetParameter(uint8_t index, std::nullptr_t)
    {
        AssertValidIndex(index);
        _paramAssignFlag[index] = true;
        MySqlBind *pParam       = &_pBind[index];
        pParam->buffer_type     = MYSQL_TYPE_NULL;
        delete[] static_cast<char *>(pParam->buffer);
        pParam->buffer        = nullptr;
        pParam->buffer_length = 0;
        pParam->is_null_value = true;
        delete pParam->length;
        pParam->length = nullptr;
    }

    void MySqlPreparedStatement::SetParameter(uint8_t index, bool value)
    {
        SetParameter(index, uint8_t(value ? 1 : 0));
    }

    void MySqlPreparedStatement::SetParameter(uint8_t index, const std::string &str)
    {
        AssertValidIndex(index);
        _paramAssignFlag[index] = true;
        MySqlBind *pParam       = &_pBind[index];
        uint32_t   length       = (uint32_t)str.size();
        pParam->buffer_type     = MYSQL_TYPE_VAR_STRING;
        delete[] static_cast<char *>(pParam->buffer);
        pParam->buffer        = new char[length];
        pParam->buffer_length = length;
        pParam->is_null_value = false;
        delete pParam->length;
        pParam->length = new unsigned long(length);

        memcpy(pParam->buffer, str.c_str(), length);
    }

    void MySqlPreparedStatement::SetParameter(uint8_t index, const std::vector<uint8_t> &value)
    {
        AssertValidIndex(index);
        _paramAssignFlag[index] = true;
        MySqlBind *pParam       = &_pBind[index];
        uint32_t   length       = (uint32_t)value.size();
        pParam->buffer_type     = MYSQL_TYPE_BLOB;
        delete[] static_cast<char *>(pParam->buffer);
        pParam->buffer        = new char[length];
        pParam->buffer_length = length;
        pParam->is_null_value = false;
        delete pParam->length;
        pParam->length = new unsigned long(length);

        memcpy(pParam->buffer, value.data(), length);
    }

    void MySqlPreparedStatement::AssertValidIndex(uint8_t index)
    {
        Assert(index < _paramCount,
               std::format("预处理语句id：{} 绑定第{}个参数错误，该语句共需{}个参数",
                           _pStmt->GetIndex(),
                           index,
                           _paramCount));

        if (_paramAssignFlag[index])
        {
            Log::Error("预处理语句id：{} 第{}个参数已经绑定", _pStmt->GetIndex(), index);
        }
    }

    void MySqlPreparedStatement::ClearParameters()
    {
        for (uint32_t i = 0; i < _paramCount; ++i)
        {
            delete[] _pBind[i].length;
            _pBind[i].length = nullptr;
            delete[] static_cast<char *>(_pBind[i].buffer);
            _pBind[i].buffer    = nullptr;
            _paramAssignFlag[i] = false;
        }
    }

    [[nodiscard]] std::string MySqlPreparedStatement::GetSqlString() const
    {
        std::string sqlString(_sqlString);
        size_t      pos = 0;
        for (const PreparedStatementData &data : _pStmt->GetParameters())
        {
            pos = sqlString.find('?', pos);

            std::string replaceStr = std::visit(
                [&](auto &&data) {
                    return PreparedStatementData::ToString(data);
                },
                data.data);
            sqlString.replace(pos, 1, replaceStr);
            pos += replaceStr.length();
        }

        return sqlString;
    }

} // namespace Database