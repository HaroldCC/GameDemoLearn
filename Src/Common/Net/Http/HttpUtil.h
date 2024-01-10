/*************************************************************************
> File Name       : HttpUtil.h
> Brief           : Http工具集
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月09日  15时49分00秒
************************************************************************/
#pragma once

#include "HttpCommon.h"

#include <string_view>

namespace Http
{
    std::string StatusToResponseHead(StatusCode status);

    std::string ContentTypeString(ContentType type, std::string_view charset = "UTF-8");

    std::string StatusToResponseContent(StatusCode statusCode);

    constexpr std::string_view HttpMethodToString(HttpMethod mtd)
    {
        switch (mtd)
        {
            case HttpMethod::Unknown:
                return "UNKNOWN";
            case HttpMethod::Delete:
                return "DELETE";
            case HttpMethod::Get:
                return "GET";
            case HttpMethod::Head:
                return "HEAD";
            case HttpMethod::Post:
                return "POST";
            case HttpMethod::Put:
                return "PUT";
            case HttpMethod::Patch:
                return "PATCH";
            case HttpMethod::Connect:
                return "CONNECT";
            case HttpMethod::Options:
                return "OPTIONS";
        };

        return {};
    }

    HttpMethod StringToHttpMethod(std::string_view mtd);
} // namespace Http