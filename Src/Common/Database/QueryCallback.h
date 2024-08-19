/*************************************************************************
> File Name       : QueryCallback.h
> Brief           : 查询回调
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月18日  15时57分12秒
************************************************************************/
#pragma once

#include "DatabaseEnv.h"
#include "QueryResult.h"
#include "Common/Util//AsyncCallbackProcessor.h"

#include <variant>
#include <queue>
#include <list>
#include <functional>

namespace Database
{
    class QueryCallback
    {
    public:
        explicit QueryCallback(QueryResultFuture &&result);
        explicit QueryCallback(PreparedQueryResultFuture &&result);
        ~QueryCallback() = default;

        QueryCallback(QueryCallback &&right) noexcept;
        QueryCallback &operator=(QueryCallback &&right) noexcept;

        QueryCallback &&Then(std::function<void(QueryResultSetPtr)> &&callback);
        QueryCallback &&Then(std::function<void(PreparedQueryResultSetPtr)> &&callback);

        QueryCallback(const QueryCallback &)            = delete;
        QueryCallback &operator=(const QueryCallback &) = delete;

        bool InvokeIfReady();

    private:
        std::variant<QueryResultFuture, PreparedQueryResultFuture> _queryFuture;
        bool                                                       _isPreparedResult;

        using QueryCallbackData = std::variant<std::function<void(QueryResultSetPtr)>,
                                               std::function<void(PreparedQueryResultSetPtr)>>;

        std::queue<QueryCallbackData, std::list<QueryCallbackData>> _callbacks;
    };

    using QueryCallbackProcessor = AsyncCallbackProcessor<QueryCallback>;
} // namespace Database