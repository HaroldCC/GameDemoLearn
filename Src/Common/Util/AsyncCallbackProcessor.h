/*************************************************************************
> File Name       : AsyncCallbackProcessor.h
> Brief           : 异步回调处理器
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月23日  11时06分23秒
************************************************************************/
#pragma once

#include <vector>

template <typename T>
concept AsyncCallback = requires(T obj) {
    {
        obj.InvokeIfReady()
    } -> std::convertible_to<bool>;
};

template <AsyncCallback Callback>
class AsyncCallbackProcessor
{
public:
    AsyncCallbackProcessor()  = default;
    ~AsyncCallbackProcessor() = default;

    AsyncCallbackProcessor(const AsyncCallbackProcessor &)            = delete;
    AsyncCallbackProcessor(AsyncCallbackProcessor &&)                 = delete;
    AsyncCallbackProcessor &operator=(const AsyncCallbackProcessor &) = delete;
    AsyncCallbackProcessor &operator=(AsyncCallbackProcessor &&)      = delete;

    Callback &AddCallback(Callback &&callback)
    {
        _callbacks.emplace_back(std::move(callback));
        return _callbacks.back();
    }

    void ProcessReadyCallbacks()
    {
        if (_callbacks.empty())
        {
            return;
        }

        std::vector<Callback> updateCallbacks {std::move(_callbacks)};
        updateCallbacks.erase(std::remove_if(updateCallbacks.begin(),
                                             updateCallbacks.end(),
                                             [](Callback &callback) {
                                                 return callback.InvokeIfReady();
                                             }),
                              updateCallbacks.end());

        _callbacks.insert(_callbacks.end(),
                          std::make_move_iterator(updateCallbacks.begin()),
                          std::make_move_iterator(updateCallbacks.end()));
    }

private:
    std::vector<Callback> _callbacks;
};