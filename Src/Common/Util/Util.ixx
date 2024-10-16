module;

#include <type_traits>
#include <string_view>
#include <cctype>
#include <optional>
#include <charconv>
#include <string>
#include <filesystem>

#include "Platform.h"
#include "Assert.h"

#ifdef OS_PLATFORM_WINDOWS
    #include <Windows.h>
#else
    #include <unistd.h>
#endif

export module Common:Util;

export namespace Util
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
        // ... (保持 detail 命名空间中的所有内容不变)
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

    inline std::filesystem::path GetExecutableDirectoryPath()
    {
#ifdef OS_PLATFORM_WINDOWS
        wchar_t path[MAX_PATH];
        if (GetModuleFileNameW(nullptr, path, MAX_PATH) == 0)
        {
            return {}; // 返回空路径表示失败
        }
#else
        char    path[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
        if (count <= 0 || count >= PATH_MAX)
        {
            return {}; // 返回空路径表示失败
        }
        path[count] = '\0';
#endif
        return std::filesystem::path(path).parent_path();
    }
} // namespace Util
