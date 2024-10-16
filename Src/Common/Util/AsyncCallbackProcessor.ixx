module;

#include <vector>
#include <concepts>
#include <algorithm>

export module Common:AsyncCallbackProcessor;

export template <typename T>
concept AsyncCallback = requires(T obj) {
    { obj.InvokeIfReady() } -> std::convertible_to<bool>;
};

export template <AsyncCallback Callback>
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
