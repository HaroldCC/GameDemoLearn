/*************************************************************************
> File Name       : QueryResult.h
> Brief           : 查询结果集
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月12日  14时40分23秒
************************************************************************/
#pragma once
#include "DatabaseEnv.h"
#include "Field.h"

#include <vector>

namespace Database
{
    class QueryResultSet final
    {
    public:
        QueryResultSet(const QueryResultSet &)            = delete;
        QueryResultSet(QueryResultSet &&)                 = delete;
        QueryResultSet &operator=(const QueryResultSet &) = delete;
        QueryResultSet &operator=(QueryResultSet &&)      = delete;

        QueryResultSet(MySqlResult *pResult, MySqlField *pFields, uint64_t rowCount, uint32_t fieldCount);
        ~QueryResultSet();

        bool NextRow();

        [[nodiscard]] uint64_t GetRowCount() const
        {
            return _rowCount;
        }

        [[nodiscard]] uint32_t GetFieldCount() const
        {
            return _fieldCount;
        }

        [[nodiscard]] Field *Fetch() const
        {
            return _pCurrentRow;
        }

        const Field &operator[](size_t index) const;

    private:
        void CleanUp();

    private:
        std::vector<QueryResultFieldMetadata> _fieldMetadata;
        Field                                *_pCurrentRow {nullptr};
        uint64_t                              _rowCount {0};
        uint32_t                              _fieldCount {0};

        MySqlResult *_pResult {nullptr};
        MySqlField  *_pFields {nullptr};
    };

    class PreparedQueryResultSet
    {
    public:
        PreparedQueryResultSet(const PreparedQueryResultSet &)            = delete;
        PreparedQueryResultSet(PreparedQueryResultSet &&)                 = delete;
        PreparedQueryResultSet &operator=(const PreparedQueryResultSet &) = delete;
        PreparedQueryResultSet &operator=(PreparedQueryResultSet &&)      = delete;

        PreparedQueryResultSet(MySqlStmt   *pStmt,
                               MySqlResult *pResult,
                               uint64_t     rowCount,
                               uint32_t     fieldCount);
        ~PreparedQueryResultSet();

        bool NextRow();

        [[nodiscard]] uint64_t GetRowCount() const
        {
            return _rowCount;
        }

        [[nodiscard]] uint32_t GetFieldCount() const
        {
            return _fieldCount;
        }

        [[nodiscard]] Field *Fetch() const;

        const Field &operator[](std::size_t index) const;

    private:
        void CleanUp();
        bool FetchNextRow();

    private:
        std::vector<QueryResultFieldMetadata> _fieldMetadata;
        std::vector<Field>                    _rows;
        uint64_t                              _rowCount {0};
        uint64_t                              _rowPosition {0};
        uint32_t                              _fieldCount {0};

        MySqlStmt   *_pStmt {nullptr};
        MySqlBind   *_pBind {nullptr};
        MySqlResult *_pMetaDataResult {nullptr};
    };
} // namespace Database