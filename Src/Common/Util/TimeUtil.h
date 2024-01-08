/*************************************************************************
> File Name       : TimeUtil.h
> Brief           : 时间工具集
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月08日  10时28分42秒
************************************************************************/
#pragma once

#include "Log.h"

#include <string_view>
#include <source_location>
#include <chrono>

namespace TimeUtil
{
    class Timer
    {
    public:
        explicit Timer(std::string_view     msg          = "",
                       bool                 logOnDestroy = false,
                       std::source_location loc          = std::source_location::current())
            : _msg(msg)
            , _sourceLoc(loc)
            , _logOnDestroy(logOnDestroy)
        {
            Reset();
        }

        Timer(const Timer &)            = default;
        Timer &operator=(const Timer &) = default;
        Timer(Timer &&)                 = delete;
        Timer &operator=(Timer &&)      = delete;

        ~Timer()
        {
            if (_logOnDestroy)
            {
                Log::Debug({"{} 耗时:{} ms",
                            Log::SourceLocation(_sourceLoc.file_name(),
                                                _sourceLoc.function_name(),
                                                _sourceLoc.line())},
                           _msg,
                           ElapsedMillisec());
            };
        }

        void Reset()

        {
            _startTimePoint = std::chrono::high_resolution_clock::now();
        }

        double ElapsedNanosec()
        {
            auto currentTimePoint = std::chrono::high_resolution_clock::now();
            return (double)std::chrono::duration_cast<std::chrono::nanoseconds>(currentTimePoint
                                                                                - _startTimePoint)
                .count();
        }

        double ElapsedMicrosec()
        {
            return ElapsedNanosec() * 0.001;
        }

        double ElapsedMillisec()
        {
            return ElapsedMicrosec() * 0.001;
        }

        double ElapsedSec()
        {
            return ElapsedMillisec() * 0.001;
        }

    private:
        std::string_view                                            _msg;
        std::chrono::time_point<std::chrono::high_resolution_clock> _startTimePoint;
        std::source_location                                        _sourceLoc;
        bool                                                        _logOnDestroy;
    };

    inline std::string GetGMTTimeStr()
    {
        return std::format("{:%a, %d %b %Y %H:%M:%OS GMT}", std::chrono::system_clock::now());
    }
} // namespace TimeUtil