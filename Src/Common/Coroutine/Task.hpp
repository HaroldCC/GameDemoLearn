/*************************************************************************
> File Name       : Task.hpp
> Brief           : 异步任务
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年05月04日  11时37分21秒
************************************************************************/
#pragma once

#include <coroutine>
#include <concepts>
#include <exception>
#include <semaphore>
#include <variant>
#include "Common/Util/Log.h"

namespace Coroutine
{

    namespace detail
    {
        class PromiseTypeBase
        {
            struct FinalAwaitable
            {
                inline constexpr bool await_ready() const noexcept
                {
                    return false;
                }

                template <std::derived_from<PromiseTypeBase> PromiseType>
                std::coroutine_handle<> await_suspend(std::coroutine_handle<PromiseType> coroutine) noexcept
                {
                    return coroutine.promise()._continueCoro;
                }

                inline constexpr void await_resume() noexcept
                {
                }
            };

        public:
            PromiseTypeBase &operator=(PromiseTypeBase &&) = delete;

            inline constexpr auto initial_suspend() noexcept
            {
                return std::suspend_always {};
            }

            inline constexpr FinalAwaitable final_suspend() noexcept
            {
                return {};
            }

            void SetContinuation(std::coroutine_handle<> coroutine) noexcept
            {
                _continueCoro = coroutine;
            }

        private:
            std::coroutine_handle<> _continueCoro;
        };

        template <typename T>
        class PromiseType final : public PromiseTypeBase
        {
        public:
            void unhandled_exception() noexcept
            {
                _result.template emplace<std::exception_ptr>(std::current_exception());
            }

            auto get_return_object() noexcept
            {
                return std::coroutine_handle<PromiseType>::from_promise(*this);
            }

            template <typename ValueType>
                requires std::convertible_to<ValueType &&, T>
            void return_value(ValueType &&value) noexcept(std::is_nothrow_constructible_v<T, ValueType &&>)
            {
                _result.template emplace<ValueType>(std::forward<ValueType>(value));
            }

            T &result() &
            {
                if (std::holds_alternative<std::exception_ptr>(_result)) [[unlikely]]
                {
                    std::rethrow_exception(std::get<std::exception_ptr>(_result));
                }

                return std::get<T>(_result);
            }

            T &&result() &&
            {
                if (std::holds_alternative<std::exception_ptr>(_result)) [[unlikely]]
                {
                    std::rethrow_exception(std::get<std::exception_ptr>(_result));
                }

                return std::move(std::get<T>(_result));
            }

        private:
            std::variant<std::monostate, T, std::exception_ptr> _result;
        };

        template <>
        class PromiseType<void> final : public PromiseTypeBase
        {
        public:
            void unhandled_exception() noexcept
            {
                _exception = std::current_exception();
            }

            auto get_return_object()
            {
                return std::coroutine_handle<PromiseType>::from_promise(*this);
            }

            constexpr void return_void() noexcept
            {
            }

            void result()
            {
                if (_exception != nullptr) [[unlikely]]
                {
                    std::rethrow_exception(_exception);
                }
            }

        private:
            std::exception_ptr _exception = nullptr;
        };
    } // namespace detail

    template <typename T = void>
    class [[nodiscard("must with keyworld co_wait")]] Task
    {
    public:
        using promise_type = detail::PromiseType<T>;
        using value_type   = T;

        Task(std::coroutine_handle<promise_type> coroutine = nullptr) noexcept : _coroutine(coroutine)
        {
        }

        Task(Task &&right) noexcept : _coroutine(right._coroutine)
        {
            right._coroutine = nullptr;
        }

        Task &operator=(Task &&right) noexcept
        {
            std::swap(_coroutine, right._coroutine);
            return *this;
        }

        Task(const Task &)            = delete;
        Task &operator=(const Task &) = delete;

        ~Task()
        {
            if (_coroutine != nullptr)
            {
                _coroutine.destroy();
            }
        }

        bool IsReady() const noexcept
        {
            return _coroutine == nullptr || _coroutine.done();
        }

        std::coroutine_handle<promise_type> get() const noexcept
        {
            return _coroutine;
        }

        std::coroutine_handle<promise_type>
        reset(std::coroutine_handle<promise_type> cotoutine = nullptr) noexcept
        {
            return std::exchange(_coroutine, cotoutine);
        }

        auto operator co_await() const & noexcept
        {
            // return TaskAwaiter {_coroutine};
            struct awaitable : TaskAwaiter
            {
                using TaskAwaiter::TaskAwaiter;

                decltype(auto) await_resume()
                {
                    if (_coroutine == nullptr)
                    {
                        Log::Error("Broken Promise");
                        throw std::logic_error("broken promise");
                    }

                    return _coroutine.promise().result();
                }
            };

            return awaitable {_coroutine};
        }

        auto operator co_await() const && noexcept
        {
            // return TaskAwaiter {_coroutine};
            struct awaitable : TaskAwaiter
            {
                using TaskAwaiter::TaskAwaiter;

                decltype(auto) await_resume()
                {
                    if (_coroutine == nullptr)
                    {
                        Log::Error("Broken Promise");
                    }

                    return std::move(_coroutine.promise().result());
                }
            };

            return awaitable {_coroutine};
        }

    private:
        struct TaskAwaiter
        {
            std::coroutine_handle<promise_type> _coroutine;

            TaskAwaiter(std::coroutine_handle<promise_type> coroutine) : _coroutine(coroutine)
            {
            }

            TaskAwaiter(TaskAwaiter &&awaiter) : _coroutine(std::exchange(awaiter._coroutine, nullptr))
            {
            }

            // ~TaskAwaiter() noexcept
            // {
            //     if (nullptr != _coroutine && _coroutine.done()) [[unlikely]]
            //     {
            //         Log::Error("协程执行完但结果未获取");
            //         _coroutine.promise().result();
            //     }
            // }

            bool await_ready() const noexcept

            {
                return _coroutine == nullptr || _coroutine.done();
            }

            std::coroutine_handle<> await_suspend(std::coroutine_handle<> awaitCoroutine) const noexcept
            {
                // 当前协程挂起的时候，记录挂起该协程的协程，作为continuation，使得当前协程执行完继续之前的协程
                _coroutine.promise().SetContinuation(awaitCoroutine);
                return _coroutine;
            }

            T await_resume()
            {
                auto coroutine = _coroutine;
                _coroutine     = nullptr;

                return coroutine.promise().result();
            }
        };

        std::coroutine_handle<promise_type> _coroutine;
    };

    template <typename TaskType>
    auto SyncWait(TaskType &&task)
    {
        std::binary_semaphore cond(0);
    }

} // namespace Coroutine