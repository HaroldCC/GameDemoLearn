/*************************************************************************
> File Name       : Util.h
> Brief           : 工具集
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月08日  09时23分16秒
************************************************************************/
#pragma once

#include "Assert.h"

#include <type_traits>
#include <string_view>
#include <cctype>
#include <optional>
#include <charconv>
#include <string>

namespace Util
{
    template <typename T>
    class Singleton
    {
    public:
        Singleton(const Singleton &)            = delete;
        Singleton(Singleton &&)                 = delete;
        Singleton &operator=(const Singleton &) = delete;
        Singleton &operator=(Singleton &&)      = delete;

        static T &Ins()
        {
            static_assert(std::is_default_constructible_v<T>, "单例需要默认构造函数");
            static T instance;
            return instance;
        }

    protected:
        Singleton()          = default;
        virtual ~Singleton() = default;
    };

    template <typename Enum>
    inline constexpr std::underlying_type_t<Enum> ToUnderlying(Enum eum) noexcept
    {
        return static_cast<std::underlying_type_t<Enum>>(eum);
    }

    inline bool StringEqual(std::string_view strLeft, std::string_view strRight)
    {
        return std::equal(strLeft.begin(),
                          strLeft.end(),
                          strRight.begin(),
                          strRight.end(),
                          [](char left, char right) {
                              return std::tolower(left) == std::tolower(right);
                          });
    }

    namespace detail
    {
        template <typename T>
            requires std::is_integral_v<T> && (!std::is_same_v<T, bool>)
        struct For
        {
            constexpr static std::optional<T> FromString(std::string_view str, int base = 10)
            {
                if (0 == base || 10 != base)
                {
                    if (Util::StringEqual(str.substr(0, 2), "0x"))
                    {
                        base = 16;
                        str.remove_prefix(2);
                    }
                    else if (Util::StringEqual(str.substr(0, 2), "0b"))
                    {
                        base = 2;
                        str.remove_prefix(2);
                    }
                    else
                    {
                        base = 10;
                    }

                    if (str.empty())
                    {
                        return std::nullopt;
                    }
                }

                const char *const begin = str.data();
                const char *const end   = (begin + str.length());
                T                 val;
                auto [ptr, errc] = std::from_chars(begin, end, val, base);
                if (ptr == end && errc == std::errc {})
                {
                    return val;
                }

                return std::nullopt;
            }

            constexpr static std::string ToString(T val)
            {
                using BufferSize = std::integral_constant<size_t, sizeof(T) < 8 ? 11 : 20>;

                std::string buf(BufferSize::value, '\0');
                char *const beg  = buf.data();
                char *const end  = (beg + buf.length());
                auto [ptr, errc] = std::to_chars(beg, end, val);
                Assert(errc == std::errc {});
                buf.resize(ptr - beg);
                return buf;
            }
        };

        struct ForBool
        {
            constexpr static std::optional<bool> FromString(std::string_view str, int strict = 0)
            {
                if (strict)
                {
                    if (str == "1")
                    {
                        return true;
                    }
                    if (str == "0")
                    {
                        return false;
                    }

                    return std::nullopt;
                }

                if ((str == "1") || StringEqual(str, "y") || StringEqual(str, "on")
                    || StringEqual(str, "yes"))
                {
                    return true;
                }

                if ((str == "0") || StringEqual(str, "n") || StringEqual(str, "off")
                    || StringEqual(str, "no"))
                {
                    return false;
                }

                return std::nullopt;
            }

            constexpr static std::string ToString(bool val)
            {
                return val ? "1" : "0";
            }
        };

        template <typename T>
            requires std::is_floating_point_v<T>
        struct ForFloat
        {
            constexpr static std::optional<T> FromString(std::string_view  str,
                                                         std::chars_format charFmt = std::chars_format {})
            {
                if (str.empty())
                {
                    return std::nullopt;
                }

                if (charFmt == std::chars_format {} || charFmt != std::chars_format::general)
                {
                    if (StringEqual(str.substr(0, 2), "0x"))
                    {
                        charFmt = std::chars_format::hex;
                        str.remove_prefix(2);
                    }
                    else
                    {
                        charFmt = std::chars_format::general;
                    }

                    if (str.empty())
                    {
                        return std::nullopt;
                    }
                }

                const char *beg = str.data();
                const char *end = (beg + str.length());
                T           val {};
                auto [ptr, errc] = std::from_chars(beg, end, val, charFmt);
                if (ptr == end && errc == std::errc {})
                {
                    return val;
                }

                return std::nullopt;
            }

            constexpr static std::optional<T> FromString(std::string_view str, int base)
            {
                if (16 == base)
                {
                    return FromString(str, std::chars_format::hex);
                }

                if (10 == base)
                {
                    return FromString(str, std::chars_format::general);
                }

                return FromString(str, std::chars_format {});
            }

            constexpr static std::string ToString(T val)
            {
                return std::format("{}", val);
            }
        };
    } // namespace detail

    template <typename Result, typename... Params>
    constexpr inline std::optional<Result> StringTo(std::string_view str, Params &&...params)
    {
        if constexpr (std::is_same_v<Result, bool>)
        {
            return detail::ForBool::FromString(str, std::forward<Params>(params)...);
        }
        else if constexpr (std::is_floating_point_v<Result>)
        {
            return detail::ForFloat<Result>::FromString(str, std::forward<Params>(params)...);
        }
        else
        {
            return detail::For<Result>::FromString(str, std::forward<Params>(params)...);
        }
    }

    template <typename Type, typename... Params>
    std::string ToString(Type &&val, Params &&...params)
    {
        if constexpr (std::is_same_v<Type, bool>)
        {
            return detail::ForBool::ToString(std::forward<Type>(val), std::forward<Params>(params)...);
        }
        else if constexpr (std::is_floating_point_v<std::decay_t<Type>>)
        {
            return detail::ForFloat<std::decay_t<Type>>::ToString(std::forward<Type>(val),
                                                                  std::forward<Params>(params)...);
        }
        else
        {
            return detail::For<std::decay_t<Type>>::ToString(std::forward<Type>(val),
                                                             std::forward<Params>(params)...);
        }
    }
} // namespace Util
