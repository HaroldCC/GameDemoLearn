/*************************************************************************
> File Name       : HttpCommon.h
> Brief           : Http公共定义
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月09日  14时39分02秒
************************************************************************/
module;
#include <cstdint>

export module Common:Net.HttpCommon;

export namespace Http
{
    enum class StatusCode : uint64_t
    {
        Unknown             = 0,
        SwitchingProtocols  = 101,
        Ok                  = 200,
        Created             = 201,
        Accepted            = 202,
        NoContent           = 204,
        PartialContent      = 206,
        MultipleChoices     = 300,
        MovedPermanently    = 301,
        MovedTemporarily    = 302,
        NotModified         = 304,
        TemporaryRedirect   = 307,
        BadRequest          = 400,
        Unauthorized        = 401,
        Forbidden           = 403,
        NotFound            = 404,
        Conflict            = 409,
        InternalServerError = 500,
        NotImplemented      = 501,
        BadGateway          = 502,
        ServiceUnavailable  = 503,

        Max,
    };

    enum class HttpMethod : uint32_t
    {
        Unknown,
        Delete,
        Get,
        Head,
        Post,
        Put,
        Patch,
        Connect,
        Options,
    };

    enum class ContentType
    {
        String,
        Html,
        Json,
    };
} // namespace Http