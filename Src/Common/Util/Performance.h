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

    #define PERFORMANCE_SCOPE_LINE(msg, line) \
        Util::TimeUtil::Timer timer##line(msg, true, std::source_location::current())
    #define PERFORMANCE_SCOPE(msg) PERFORMANCE_SCOPE_LINE(msg, __LINE__)
    #define PERFORMANCE_SCOPE(msg)
#else
    #define PERFORMANCE_SCOPE(msg)
#endif