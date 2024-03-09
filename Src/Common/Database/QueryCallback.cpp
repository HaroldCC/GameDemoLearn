/*************************************************************************
> File Name       : QueryCallback.cpp
> Brief           : 查询回调
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月18日  16时24分36秒
************************************************************************/
#include "QueryCallback.h"
#include "Common/Util/Assert.h"

#include <functional>

#define ASIO_HAS_CO_AWAIT
#include <asio/awaitable.hpp>

namespace Database
{

    QueryCallback::QueryCallback(QueryResultFuture &&result)
        : _future(std::move(result))
        , _isPreparedResult(false)
    {
    }

    QueryCallback::QueryCallback(PreparedQueryResultFuture &&result)
        : _future(std::move(result))
        , _isPreparedResult(true)
    {
    }

    QueryCallback::QueryCallback(QueryCallback&& right) noexcept {
        using std::swap;
        swap(_future, right._future);
        swap(_isPreparedResult, right._isPreparedResult);
    }

    QueryCallback& QueryCallback::operator=(QueryCallback&& right) noexcept {
        using std::swap;
        swap(_future, right._future);
        swap(_isPreparedResult, right._isPreparedResult);
        return *this;
    }


    QueryCallback &&QueryCallback::Then(std::function<void(QueryResultSetPtr)> &&callback)
    {
        _callbacks.emplace(std::move(callback));
        return std::move(*this);
    }

    QueryCallback &&QueryCallback::Then(std::function<void(PreparedQueryResultSetPtr)> &&callback)
    {
        _callbacks.emplace(std::move(callback));
        return std::move(*this);
    }

    bool QueryCallback::InvokeIfReady()
    {
        QueryCallbackData &callback                    = _callbacks.front();
        auto               checkStateAndReturnComplete = [this]() {
            _callbacks.pop();
            bool bFinish = std::visit(
                [](auto &&future) -> bool {
                    return future.valid();
                },
                _future);

            if (_callbacks.empty())
            {
                Assert(!bFinish) return true;
            }

            return false;
        };

        if (_isPreparedResult)
        {
            PreparedQueryResultFuture *pFuture = std::get_if<PreparedQueryResultFuture>(&_future);
            Assert(nullptr != pFuture);

            if (pFuture->valid() && pFuture->wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                PreparedQueryResultFuture future(std::move(*pFuture));
                using CallbackType      = std::function<void(PreparedQueryResultSetPtr)>;
                CallbackType *pCallback = std::get_if<CallbackType>(&callback._callback);
                Assert(nullptr == pCallback);

                (*pCallback)(future.get());
                return checkStateAndReturnComplete();
            }
        }
        else
        {
            QueryResultFuture *pFuture = std::get_if<QueryResultFuture>(&_future);
            Assert(nullptr != pFuture);

            if (pFuture->valid() && pFuture->wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                QueryResultFuture future(std::move(*pFuture));
                using CallbackType      = std::function<void(QueryResultSetPtr)>;
                CallbackType *pCallback = std::get_if<CallbackType>(&callback._callback);
                Assert(nullptr != pCallback);

                (*pCallback)(future.get());
                return checkStateAndReturnComplete();
            }
        }

        return false;
    }

} // namespace Database