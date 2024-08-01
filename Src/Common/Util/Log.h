/*************************************************************************
> File Name       : Log.h
> Brief           : 日志模块
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月05日  17时32分49秒
************************************************************************/
#pragma once
#define SPDLOG_SOURCE_LOCATION
#include "spdlog/spdlog.h"

#include <string_view>

constexpr inline std::string_view GetDefaultLogPattern()
{
#if defined(_DEBUG) || defined(DEBUG)
    return "%^[%Y-%m-%d %T%e] [%s:%# %l]: %v%$";
#else
    return "%^[%Y-%m-%d %T%e] %l: %v%$";
#endif
}

namespace Log
{
    class CLogger final
    {
    public:
        static CLogger &GetLogger()
        {
            static CLogger logger;
            return logger;
        }

        CLogger(const CLogger &)            = delete;
        CLogger &operator=(const CLogger &) = delete;
        CLogger(CLogger &&)                 = delete;
        CLogger &operator=(CLogger &&)      = delete;

        void InitLogger(std::string_view fileName,
                        size_t           level,
                        size_t           maxFileSize,
                        size_t           maxFiles,
                        std::string_view pattern = GetDefaultLogPattern());

        void Flush() const
        {
            _logger->flush();
        }

    private:
        CLogger() = default;

        ~CLogger() = default;

    private:
        std::shared_ptr<spdlog::logger> _logger;
    };

    template <typename... Args>
    inline void Tracy(spdlog::loc_with_fmt fmt, Args &&...args)
    {
        spdlog::log(fmt.loc, spdlog::level::trace, fmt.fmt_string, std::forward<Args>(args)...);
    }

    template <typename... Args>
    inline void Debug(spdlog::loc_with_fmt fmt, Args &&...args)
    {
        spdlog::log(fmt.loc, spdlog::level::debug, fmt.fmt_string, std::forward<Args>(args)...);
    }

    template <typename... Args>
    inline void Info(spdlog::loc_with_fmt fmt, Args &&...args)
    {
        spdlog::log(fmt.loc, spdlog::level::info, fmt.fmt_string, std::forward<Args>(args)...);
    }

    template <typename... Args>
    inline void Warn(spdlog::loc_with_fmt fmt, Args &&...args)
    {
        spdlog::log(fmt.loc, spdlog::level::warn, fmt.fmt_string, std::forward<Args>(args)...);
    }

    template <typename... Args>
    inline void Error(spdlog::loc_with_fmt fmt, Args &&...args)
    {
        spdlog::log(fmt.loc, spdlog::level::err, fmt.fmt_string, std::forward<Args>(args)...);
    }

    template <typename... Args>
    inline void Critical(spdlog::loc_with_fmt fmt, Args &&...args)
    {
        spdlog::log(fmt.loc, spdlog::level::critical, fmt.fmt_string, std::forward<Args>(args)...);
    }

} // namespace Log
