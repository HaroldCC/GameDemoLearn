/*************************************************************************
> File Name       : IOContextPool.cpp
> Brief           : IO池
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年08月15日  17时25分02秒
************************************************************************/
#include "IOContextPool.h"

namespace Util
{
    IOContextPool::IOContextPool(std::size_t poolSize) : _nextIOContext(0)
    {
        if (poolSize == 0)
        {
            poolSize = 1;
        }

        _totalThreadCount += poolSize;

        for (std::size_t i = 0; i < poolSize; ++i)
        {
            IOContextPtr pIOContext = std::make_shared<asio::io_context>(1);
            WorkPtr      pWork      = std::make_shared<asio::io_context::work>(*pIOContext);
            _ioContexts.emplace_back(pIOContext);
            _works.emplace_back(pWork);
        }
    }

    void IOContextPool::Run()
    {
        bool isRun = false;
        bool ok    = _isRun.compare_exchange_strong(isRun, true);
        if (!ok)
        {
            return;
        }

        _works.clear();

        if (ok)
        {
            for (auto &pCtx : _ioContexts)
            {
                pCtx->run();
            }
            return;
        }
    }

    void IOContextPool::Stop()
    {
    }

    // asio::io_context &IOContextPool::GetIOContext()
    // {
    // }
} // namespace Util