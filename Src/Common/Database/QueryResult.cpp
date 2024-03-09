/*************************************************************************
> File Name       : QueryResult.cpp
> Brief           : 查询结果集
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月12日  14时54分09秒
************************************************************************/
#include "QueryResult.h"
#include "Field.h"
#include "MySqlTypeHack.h"
#include "Common/Util/Assert.h"

namespace Database
{
    constexpr uint32_t SizeForType(MYSQL_FIELD *field)
    {
        Assert(nullptr != field);

        switch (field->type)
        {
            case MYSQL_TYPE_NULL:
                return 0;
            case MYSQL_TYPE_TINY:
                return 1;
            case MYSQL_TYPE_YEAR:
            case MYSQL_TYPE_SHORT:
                return 2;
            case MYSQL_TYPE_INT24:
            case MYSQL_TYPE_LONG:
            case MYSQL_TYPE_FLOAT:
                return 4;
            case MYSQL_TYPE_DOUBLE:
            case MYSQL_TYPE_LONGLONG:
            case MYSQL_TYPE_BIT:
                return 8;

            case MYSQL_TYPE_TIMESTAMP:
            case MYSQL_TYPE_DATE:
            case MYSQL_TYPE_TIME:
            case MYSQL_TYPE_DATETIME:
                return sizeof(MYSQL_TIME);

            case MYSQL_TYPE_TINY_BLOB:
            case MYSQL_TYPE_MEDIUM_BLOB:
            case MYSQL_TYPE_LONG_BLOB:
            case MYSQL_TYPE_BLOB:
            case MYSQL_TYPE_STRING:
            case MYSQL_TYPE_VAR_STRING:
                return field->max_length + 1;

            case MYSQL_TYPE_DECIMAL:
            case MYSQL_TYPE_NEWDECIMAL:
                return 64;

            case MYSQL_TYPE_GEOMETRY:
                /*
        Following types are not sent over the wire:
        MYSQL_TYPE_ENUM:
        MYSQL_TYPE_SET:
        */
            default:
                Log::Error("invalid field type {}", uint32_t(field->type));
                return 0;
        }
    }

    constexpr DatabaseFieldType MysqlTypeToFieldType(enum_field_types type, uint32_t flags)
    {
        switch (type)
        {
            case MYSQL_TYPE_NULL:
                return DatabaseFieldType::Null;
            case MYSQL_TYPE_TINY:
                return (flags & UNSIGNED_FLAG) ? DatabaseFieldType::UInt8 : DatabaseFieldType::Int8;
            case MYSQL_TYPE_YEAR:
            case MYSQL_TYPE_SHORT:
                return (flags & UNSIGNED_FLAG) ? DatabaseFieldType::UInt16 : DatabaseFieldType::Int16;
            case MYSQL_TYPE_INT24:
            case MYSQL_TYPE_LONG:
                return (flags & UNSIGNED_FLAG) ? DatabaseFieldType::UInt32 : DatabaseFieldType::Int32;
            case MYSQL_TYPE_LONGLONG:
            case MYSQL_TYPE_BIT:
                return (flags & UNSIGNED_FLAG) ? DatabaseFieldType::UInt64 : DatabaseFieldType::Int64;
            case MYSQL_TYPE_FLOAT:
                return DatabaseFieldType::Float;
            case MYSQL_TYPE_DOUBLE:
                return DatabaseFieldType::Double;
            case MYSQL_TYPE_DECIMAL:
            case MYSQL_TYPE_NEWDECIMAL:
                return DatabaseFieldType::Decimal;
            case MYSQL_TYPE_TIMESTAMP:
            case MYSQL_TYPE_DATE:
            case MYSQL_TYPE_TIME:
            case MYSQL_TYPE_DATETIME:
                return DatabaseFieldType::Date;
            case MYSQL_TYPE_TINY_BLOB:
            case MYSQL_TYPE_MEDIUM_BLOB:
            case MYSQL_TYPE_LONG_BLOB:
            case MYSQL_TYPE_BLOB:
            case MYSQL_TYPE_STRING:
            case MYSQL_TYPE_VAR_STRING:
                return DatabaseFieldType::Binary;
            default:
                Log::Error("invalid field type {}", uint32_t(type));
                break;
        }

        return DatabaseFieldType::Null;
    }

    constexpr const char *FieldTypeToString(enum_field_types type, uint32_t flags)
    {
        switch (type)
        {
            case MYSQL_TYPE_BIT:
                return "BIT";
            case MYSQL_TYPE_BLOB:
                return "BLOB";
            case MYSQL_TYPE_DATE:
                return "DATE";
            case MYSQL_TYPE_DATETIME:
                return "DATETIME";
            case MYSQL_TYPE_NEWDECIMAL:
                return "NEWDECIMAL";
            case MYSQL_TYPE_DECIMAL:
                return "DECIMAL";
            case MYSQL_TYPE_DOUBLE:
                return "DOUBLE";
            case MYSQL_TYPE_ENUM:
                return "ENUM";
            case MYSQL_TYPE_FLOAT:
                return "FLOAT";
            case MYSQL_TYPE_GEOMETRY:
                return "GEOMETRY";
            case MYSQL_TYPE_INT24:
                return (flags & UNSIGNED_FLAG) ? "UNSIGNED INT24" : "INT24";
            case MYSQL_TYPE_LONG:
                return (flags & UNSIGNED_FLAG) ? "UNSIGNED LONG" : "LONG";
            case MYSQL_TYPE_LONGLONG:
                return (flags & UNSIGNED_FLAG) ? "UNSIGNED LONGLONG" : "LONGLONG";
            case MYSQL_TYPE_LONG_BLOB:
                return "LONG_BLOB";
            case MYSQL_TYPE_MEDIUM_BLOB:
                return "MEDIUM_BLOB";
            case MYSQL_TYPE_NEWDATE:
                return "NEWDATE";
            case MYSQL_TYPE_NULL:
                return "NULL";
            case MYSQL_TYPE_SET:
                return "SET";
            case MYSQL_TYPE_SHORT:
                return (flags & UNSIGNED_FLAG) ? "UNSIGNED SHORT" : "SHORT";
            case MYSQL_TYPE_STRING:
                return "STRING";
            case MYSQL_TYPE_TIME:
                return "TIME";
            case MYSQL_TYPE_TIMESTAMP:
                return "TIMESTAMP";
            case MYSQL_TYPE_TINY:
                return (flags & UNSIGNED_FLAG) ? "UNSIGNED TINY" : "TINY";
            case MYSQL_TYPE_TINY_BLOB:
                return "TINY_BLOB";
            case MYSQL_TYPE_VAR_STRING:
                return "VAR_STRING";
            case MYSQL_TYPE_YEAR:
                return "YEAR";
            default:
                return "-Unknown-";
        }
    }

    constexpr void InitializeDatabaseFieldMetadata(QueryResultFieldMetadata *meta,
                                                   const MySqlField         *field,
                                                   uint32_t                  fieldIndex,
                                                   bool                      bBinary)
    {
        Assert(nullptr != meta || nullptr != field);

        meta->tableName  = field->org_table;
        meta->tableAlias = field->table;
        meta->name       = field->org_name;
        meta->alias      = field->name;
        meta->typeName   = FieldTypeToString(field->type, field->flags);
        meta->index      = fieldIndex;
        meta->fieldType  = MysqlTypeToFieldType(field->type, field->flags);
        meta->bBinary    = bBinary;
    }

    QueryResultSet::QueryResultSet(MySqlResult *pResult,
                                   MySqlField  *pFields,
                                   uint64_t     rowCount,
                                   uint32_t     fieldCount)
        : _rowCount(rowCount)
        , _fieldCount(fieldCount)
        , _pResult(pResult)
        , _pFields(pFields)

    {
        _fieldMetadata.resize(_fieldCount);
        _pCurrentRow = new Field[_fieldCount];
        for (uint32_t i = 0; i < _fieldCount; ++i)
        {
            InitializeDatabaseFieldMetadata(&_fieldMetadata[i], &pFields[i], i, false);
            _pCurrentRow[i].SetMetadata(&_fieldMetadata[i]);
        }
    }

    QueryResultSet::~QueryResultSet()
    {
        CleanUp();
    }

    bool QueryResultSet::NextRow()
    {
        if (_pResult)
        {
            return false;
        }

        MYSQL_ROW row = nullptr;
        row           = mysql_fetch_row(_pResult);
        if (nullptr == row)
        {
            CleanUp();
            return false;
        }

        unsigned long *pLength = mysql_fetch_lengths(_pResult);
        if (nullptr == pLength)
        {
            Log::Warn("mysql_fetch_lengths获取字段长度错误：{}", mysql_errno(_pResult->handle));
            CleanUp();
            return false;
        }

        for (uint32_t i = 0; i < _fieldCount; ++i)
        {
            _pCurrentRow[i].SetValue(row[i], pLength[i]);
        }
        return true;
    }

    const Field &QueryResultSet::operator[](size_t index) const
    {
        Assert(index < _fieldCount);
        return _pCurrentRow[index];
    }

    void QueryResultSet::CleanUp()
    {
        delete[] _pCurrentRow;
        _pCurrentRow = nullptr;

        if (nullptr != _pResult)
        {
            mysql_free_result(_pResult);
            _pResult = nullptr;
        }
    }

    PreparedQueryResultSet::PreparedQueryResultSet(MySqlStmt   *pStmt,
                                                   MySqlResult *pResult,
                                                   uint64_t     rowCount,
                                                   uint32_t     fieldCount)
        : _rowCount(rowCount)
        , _fieldCount(fieldCount)
        , _pStmt(pStmt)
        , _pMetaDataResult(pResult)
    {
        if (nullptr == _pMetaDataResult)
        {
            return;
        }

        if (_pStmt->bind_result_done)
        {
            delete[] _pStmt->bind->length;
            delete[] _pStmt->bind->is_null;
        }

        _pBind = new MySqlBind[_fieldCount];

        MySqlBool     *pIsNull  = new MySqlBool[_fieldCount];
        unsigned long *pLengths = new unsigned long[_fieldCount];

        memset(_pBind, 0, sizeof(MySqlBind) * _fieldCount);
        memset(pIsNull, 0, sizeof(MySqlBool) * _fieldCount);
        memset(pLengths, 0, sizeof(unsigned long) * _fieldCount);

        if (0 != mysql_stmt_store_result(_pStmt))
        {
            Log::Warn("mysql_stmt_store_result 存储结果集错误：{}", mysql_stmt_error(_pStmt));
            delete[] _pBind;
            delete[] pIsNull;
            delete[] pLengths;

            _pBind = nullptr;
            return;
        }

        _rowCount = mysql_stmt_num_rows(_pStmt);

        MySqlField *pFields = mysql_fetch_fields(_pMetaDataResult);
        _fieldMetadata.resize(_fieldCount);
        size_t rowSize = 0;
        for (uint32_t i = 0; i < _fieldCount; ++i)
        {
            uint32_t size = SizeForType(&pFields[i]);
            rowSize += size;

            InitializeDatabaseFieldMetadata(&_fieldMetadata[i], &pFields[i], i, true);

            _pBind[i].buffer_type   = pFields[i].type;
            _pBind[i].buffer_length = size;
            _pBind[i].length        = &pLengths[i];
            _pBind[i].is_null       = &pIsNull[i];
            _pBind[i].error         = nullptr;
            _pBind[i].is_unsigned   = pFields[i].flags & UNSIGNED_FLAG;
        }

        char *pDataBuffer = new char[rowSize * _rowCount];
        for (uint32_t i = 0, offset = 0; i < _fieldCount; ++i)
        {
            _pBind[i].buffer = pDataBuffer + offset;
            offset += _pBind[i].buffer_length;
        }

        if (mysql_stmt_bind_result(_pStmt, _pBind))
        {
            Log::Warn("mysql_stmt_bind_result 绑定结果集错误：{}", mysql_stmt_errno(_pStmt));
            mysql_stmt_free_result(_pStmt);
            CleanUp();
            delete[] pIsNull;
            delete[] pLengths;
            return;
        }

        _rows.resize(_rowCount * _fieldCount);
        while (FetchNextRow())
        {
            for (uint32_t i = 0; i < _fieldCount; ++i)
            {
                _rows[_rowPosition * _fieldCount + i].SetMetadata(&_fieldMetadata[i]);

                unsigned long bufferLength  = _pBind[i].buffer_length;
                unsigned long fetchedLength = *_pBind[i].length;
                if (!(*_pBind[i].is_null))
                {
                    void *buffer = _pStmt->bind[i].buffer;
                    switch (_pBind[i].buffer_type)
                    {
                        case MYSQL_TYPE_TINY_BLOB:
                        case MYSQL_TYPE_MEDIUM_BLOB:
                        case MYSQL_TYPE_LONG_BLOB:
                        case MYSQL_TYPE_BLOB:
                        case MYSQL_TYPE_VAR_STRING:
                        case MYSQL_TYPE_STRING:
                        {
                            if (fetchedLength < bufferLength)
                            {
                                *((char *)buffer + fetchedLength) = '\0';
                            }
                        }
                        break;
                        default:
                            break;
                    }

                    _rows[_rowPosition * _fieldCount + i].SetValue((const char *)buffer, fetchedLength);

                    // 偏移指针
                    _pStmt->bind[i].buffer = (char *)buffer + rowSize;
                }
                else
                {
                    _rows[_rowPosition * _fieldCount + i].SetValue(nullptr, *_pBind[i].length);
                }
            }
            ++_rowPosition;
        }
        _rowPosition = 0;

        // 所有数据绑定完毕，释放
        mysql_stmt_free_result(_pStmt);
    }

    PreparedQueryResultSet::~PreparedQueryResultSet()
    {
        CleanUp();
    }

    bool PreparedQueryResultSet::NextRow()
    {
        return ++_rowPosition < _rowCount;
    }

    [[nodiscard]] Field *PreparedQueryResultSet::Fetch() const
    {
        Assert(_rowPosition < _rowCount);
        return const_cast<Field *>(&_rows[_rowPosition * _fieldCount]);
    }

    const Field &PreparedQueryResultSet::operator[](std::size_t index) const
    {
        Assert(_rowPosition < _rowCount);
        Assert(index < _fieldCount);
        return _rows[_rowPosition * _fieldCount + index];
    }

    void PreparedQueryResultSet::CleanUp()
    {
        if (nullptr != _pMetaDataResult)
        {
            mysql_free_result(_pMetaDataResult);
        }

        if (nullptr != _pBind)
        {
            delete[] (char *)_pBind->buffer;
            delete[] _pBind;
            _pBind = nullptr;
        }
    }

    bool PreparedQueryResultSet::FetchNextRow()
    {
        if (_rowPosition >= _rowCount)
        {
            return false;
        }

        int retVal = mysql_stmt_fetch(_pStmt);
        return retVal == 0 || retVal == MYSQL_DATA_TRUNCATED;
    }
} // namespace Database