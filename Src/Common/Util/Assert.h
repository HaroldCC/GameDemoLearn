/*************************************************************************
> File Name       : Assert.h
> Brief           : 断言
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月08日  09时24分22秒
************************************************************************/
#pragma once

#include "Platform.h"

#include <filesystem>

#if defined(DEBUG) || defined(_DEBUG)
    #if defined(OS_PLATFORM_WINDOWS)
        #define TERMINATE() __debugbreak()
    #elif defined(OS_PLATFORM_LINUX)
        #include <signal.h>
        #define TERMINATE() raise(SIGTRAP)
    #else
        #error "Platform doest't support debug break yet!"
    #endif
#else
// !!!!! todo 是否要这样？？？
    #define TERMINATE() std::abort();
#endif

#define EXPAND_MACRO(x)    x
#define STRINGIFY_MACRO(x) #x

#define INTERNAL_ASSERT_IMPL(type, check, msg, ...) \
    {                                               \
        if (!(check))                               \
        {                                           \
            Log::Critical(msg, __VA_ARGS__);        \
            Log::CLogger::GetLogger().Flush();      \
            TERMINATE();                            \
        }                                           \
    }
#define INTERNAL_ASSERT_WITH_MSG(type, check, ...) \
    INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
#define INTERNAL_ASSERT_NO_MSG(type, check)                                   \
    INTERNAL_ASSERT_IMPL(type,                                                \
                         check,                                               \
                         "Assertion '{0}' failed at {1}:{2}",                 \
                         STRINGIFY_MACRO(check),                              \
                         std::filesystem::path(__FILE__).filename().string(), \
                         __LINE__)

#define INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
#define INTERNAL_ASSERT_GET_MACRO(...) \
    EXPAND_MACRO(                      \
        INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, INTERNAL_ASSERT_WITH_MSG, INTERNAL_ASSERT_NO_MSG))

// Currently accepts at least the condition and one additional parameter (the message) being optional
#define Assert(...) EXPAND_MACRO(INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__))

// Usage:
//      1、Assert(false, "这是一个断言");
//      2、Assert(nullptr != pPointer);