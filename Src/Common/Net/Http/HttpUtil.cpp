/*************************************************************************
> File Name       : HttpUtil.cpp
> Brief           : Http工具集
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月09日  15时51分59秒
************************************************************************/
#include "HttpUtil.h"
#include "Common/Util/Util.h"
#include "magic_enum/magic_enum.hpp"

#include <format>

using namespace std::string_view_literals;

template <>
struct magic_enum::customize::enum_range<Http::StatusCode>
{
    constexpr static int min = 0;
    constexpr static int max = static_cast<int>(Http::StatusCode::Max);
};

namespace Http
{
    std::string StatusToResponseHead(StatusCode status)
    {
        return std::format("HTTP/1.1 {} {}\r\n", Util::ToUnderlying(status), magic_enum::enum_name(status));
    }

    std::string ContentTypeString(ContentType type, std::string_view charset /*= "UTF-8"*/)
    {
        switch (type)
        {
            case ContentType::String:
                return std::format("Content-Type: text/plain; charset={}\r\n", charset);
            case ContentType::Html:
                return std::format("Content-Type: text/html; charset={}\r\n", charset);
            case ContentType::Json:
                return std::format("Content-Type: application/json; charset={}\r\n", charset);
                break;
        }

        return {};
    }

    std::string StatusToResponseContent(StatusCode statusCode)
    {
        return std::format(R"(
                <html>
                <head><title>{}</title></head>
                <body><h1>{} {}</h1></body>
                </html>)",
                           magic_enum::enum_name(statusCode),
                           Util::ToUnderlying(statusCode),
                           magic_enum::enum_name(statusCode));
    }

    HttpMethod StringToHttpMethod(std::string_view mtd)
    {
        if (mtd.size() < 3)
        {
            return HttpMethod::Unknown;
        }

        char chr = mtd[0];
        mtd.remove_prefix(1);
        switch (chr)
        {
            case 'C':
            {
                if (mtd == "ONNECT")
                {
                    return HttpMethod::Connect;
                }
            }
            break;
            case 'D':
            {
                if (mtd == "ELETE")
                {
                    return HttpMethod::Delete;
                }
            }
            break;
            case 'G':
            {
                if (mtd == "ET")
                {
                    return HttpMethod::Get;
                }
            }
            break;
            case 'H':
            {
                if (mtd == "EAD")
                {
                    return HttpMethod::Head;
                }
            }
            break;
            case 'O':
            {
                if (mtd == "PTIONS")
                {
                    return HttpMethod::Options;
                }
            }
            break;
            case 'P':
            {
                chr = mtd[0];
                mtd.remove_prefix(1);
                switch (chr)
                {
                    case 'A':
                    {
                        if (mtd == "TCH")
                        {
                            return HttpMethod::Patch;
                        }
                    }
                    break;

                    case 'O':
                    {
                        if (mtd == "ST")
                        {
                            return HttpMethod::Post;
                        }
                    }
                    break;
                    case 'U':
                    {
                        if (mtd == "T")
                        {
                            return HttpMethod::Put;
                        }
                    }
                    break;
                    default:
                        break;
                }
            }
            break;
            default:
                break;
        }

        return HttpMethod::Unknown;
    }
} // namespace Http