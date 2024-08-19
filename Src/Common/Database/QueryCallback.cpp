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

namespace Database
{

    QueryCallback::QueryCallback(QueryResultFuture &&result) : _queryFuture(std::move(result))
    {
    }

    QueryCallback::QueryCallback(PreparedQueryResultFuture &&result) : _queryFuture(std::move(result))
    {
    }

    QueryCallback::QueryCallback(QueryCallback &&right) noexcept
        : _queryFuture(std::move(right._queryFuture))
        , _callbacks(std::move(right._callbacks))
    {
        //using std::swap;
        //swap(_queryFuture, right._queryFuture);
        //swap(_callbacks, right._callbacks);
    }

    QueryCallback &QueryCallback::operator=(QueryCallback &&right) noexcept
    {
        if (this != &right)
        {
            _queryFuture = std::move(right._queryFuture);
            _callbacks   = std::move(right._callbacks);
        }
        // using std::swap;
        // swap(_queryFuture, right._queryFuture);
        // swap(_callbacks, right._callbacks);
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
        auto checkStateAndReturnComplete = [this]() {
            _callbacks.pop();
            bool bFinish = std::visit(
                [](auto &&future) -> bool {
                    return future.valid();
                },
                _queryFuture);

            if (_callbacks.empty())
            {
                Assert(!bFinish);
                return true;
            }

            if (!bFinish)
            {
                return true;
            }

            Assert(_queryFuture.index() == _callbacks.front().index());
            return false;
        };

        return std::visit(
            [&]<typename Result>(std::future<Result> &&future) {
                using namespace std::chrono_literals;
                if (future.valid() && future.wait_for(0s) == std::future_status::ready)
                {
                    std::future<Result>         f(std::move(future));
                    std::function<void(Result)> cb(
                        std::get<std::function<void(Result)>>(std::move(_callbacks.front())));
                    cb(f.get());
                    return checkStateAndReturnComplete();
                }
                return false;
            },
            std::move(_queryFuture));
    }

} // namespace Database