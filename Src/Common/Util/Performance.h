/*************************************************************************
> File Name       : Performance.h
> Brief           : 性能探测
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月08日  10时31分22秒
************************************************************************/
#pragma once

#if defined(ENABLE_PERFORMANCE_DECT)
    #include "TimeUtil.h"
    #include <source_location>
    #include <format>

    #define PERFORMANCE_SCOPE_LINE(line, msg, ...) \
        TimeUtil::Timer timer##line(std::format(msg, __VA_ARGS__), true, std::source_location::current())
    #define PERFORMANCE_SCOPE(msg, ...) PERFORMANCE_SCOPE_LINE(__LINE__, msg, __VA_ARGS__)

#else
    #define PERFORMANCE_SCOPE(msg, ...)
#endif