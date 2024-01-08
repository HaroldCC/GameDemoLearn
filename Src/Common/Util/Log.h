/*************************************************************************
> File Name       : Log.h
> Brief           : 日志模块
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月05日  17时32分49秒
************************************************************************/
#pragma once
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
    class SourceLocation
    {
    public:
        static constexpr SourceLocation Current(const char *fileName = __builtin_FILE(),
                                                const char *funcName = __builtin_FUNCTION(),
                                                uint32_t    lineNum  = __builtin_LINE())
        {
            return {fileName, funcName, lineNum};
        }

        constexpr SourceLocation(const char *fileName = __builtin_FILE(),
                                 const char *funcName = __builtin_FUNCTION(),
                                 uint32_t    lineNum  = __builtin_LINE()) noexcept
            : _fileName(fileName)
            , _funcName(funcName)
            , _lineNum(lineNum)
        {
        }

        [[nodiscard]] constexpr const char *FileName() const noexcept
        {
            return _fileName;
        }

        [[nodiscard]] constexpr const char *FuncName() const noexcept
        {
            return _funcName;
        }

        [[nodiscard]] constexpr std::uint32_t LineNum() const noexcept
        {
            return _lineNum;
        }

        inline operator spdlog::source_loc()
        {
            return {_fileName, static_cast<int>(_lineNum), _funcName};
        }

    private:
        const char *_fileName {nullptr};
        const char *_funcName {nullptr};
        uint32_t    _lineNum {0};
    };

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

    struct FormatWithLocation
    {
        std::string_view format;
        SourceLocation   location;

        template <typename String>
        constexpr FormatWithLocation(String &&fmt, SourceLocation loc = SourceLocation::Current())
            : format(std::forward<String>(fmt))
            , location(loc)
        {
        }
    };

    template <typename... Args>
    inline void Tracy(FormatWithLocation fmt, Args &&...args)
    {
        spdlog::log(fmt.location,
                    spdlog::level::level_enum::trace,
                    fmt::runtime(fmt.format),
                    std::forward<Args>(args)...);
    }

    template <typename... Args>
    inline void Debug(FormatWithLocation fmt, Args &&...args)
    {
        spdlog::log(fmt.location,
                    spdlog::level::level_enum::debug,
                    fmt::runtime(fmt.format),
                    std::forward<Args>(args)...);
    }

    template <typename... Args>
    inline void Info(FormatWithLocation fmt, Args &&...args)
    {
        spdlog::log(fmt.location,
                    spdlog::level::level_enum::info,
                    fmt::runtime(fmt.format),
                    std::forward<Args>(args)...);
    }

    template <typename... Args>
    inline void Warn(FormatWithLocation fmt, Args &&...args)
    {
        spdlog::log(fmt.location,
                    spdlog::level::level_enum::warn,
                    fmt::runtime(fmt.format),
                    std::forward<Args>(args)...);
    }

    template <typename... Args>
    inline void Error(FormatWithLocation fmt, Args &&...args)
    {
        spdlog::log(fmt.location,
                    spdlog::level::level_enum::err,
                    fmt::runtime(fmt.format),
                    std::forward<Args>(args)...);
    }

    template <typename... Args>
    inline void Critical(FormatWithLocation fmt, Args &&...args)
    {
        spdlog::log(fmt.location,
                    spdlog::level::level_enum::critical,
                    fmt::runtime(fmt.format),
                    std::forward<Args>(args)...);
    }

} // namespace Log
