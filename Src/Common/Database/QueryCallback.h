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
        std::variant<QueryResultFuture, PreparedQueryResultFuture> _future;
        bool                                                       _isPreparedResult;

        struct QueryCallbackData
        {
            QueryCallbackData(const QueryCallbackData &)            = delete;
            QueryCallbackData &operator=(const QueryCallbackData &) = delete;

            QueryCallbackData() = default;

            explicit QueryCallbackData(std::function<void(QueryResultSetPtr)> &&callback)
                : _callback(std::move(callback))
                , _isPreparedResult(false)
            {
            }

            explicit QueryCallbackData(std::function<void(PreparedQueryResultSetPtr)> &&callback)
                : _callback(std::move(callback))
                , _isPreparedResult(true)
            {
            }

            QueryCallbackData(QueryCallbackData &&right) noexcept
                : _callback(std::move(right._callback))
                , _isPreparedResult(right._isPreparedResult)
            {
            }

            QueryCallbackData &operator=(QueryCallbackData &&right) noexcept
            {
                using std::swap;
                swap(_callback, right._callback);
                swap(_isPreparedResult, right._isPreparedResult);

                return *this;
            }

            std::variant<std::function<void(QueryResultSetPtr)>,
                         std::function<void(PreparedQueryResultSetPtr)>>
                 _callback;
            bool _isPreparedResult;
        };

        std::queue<QueryCallbackData> _callbacks;
    };

    using QueryCallbackProcessor = AsyncCallbackProcessor<QueryCallback>;
} // namespace Database