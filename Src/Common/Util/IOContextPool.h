/*************************************************************************
> File Name       : IOContextPool.h
> Brief           : IO工作池
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年08月15日  17时06分32秒
************************************************************************/
#pragma once

#include "asio.hpp"

#include <cstddef>
#include <memory>

namespace Util
{
    class IOContextPool final
    {
    public:
        explicit IOContextPool(std::size_t poolSize);

        void Run();
        void Stop();

        std::size_t PoolSize() const noexcept
        {
            return _ioContexts.size();
        }

        bool Empty() const noexcept
        {
            return _works.empty();
        }

        // asio::io_context &GetIOContext();

    private:
        using IOContextPtr = std::shared_ptr<asio::io_context>;
        using WorkPtr      = std::shared_ptr<asio::io_context::work>;

        std::vector<IOContextPtr>              _ioContexts;
        std::vector<WorkPtr>                   _works;
        std::atomic<std::size_t>               _nextIOContext    = 0;
        std::atomic_bool                       _isRun            = false;
        inline static std::atomic<std::size_t> _totalThreadCount = 0;
    };
} // namespace Util