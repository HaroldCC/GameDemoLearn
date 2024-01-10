/*************************************************************************
> File Name       : HttpParser.h
> Brief           : Http解析
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月09日  14时42分10秒
************************************************************************/
#pragma once

#include <string_view>
#include <array>
#include <unordered_map>

namespace Http
{
    class HttpParser final
    {
    public:
        size_t ParseRequest(std::string_view originalUrl);
        size_t ParseResponse(std::string_view originalUrl);

        void                           ParseQuery(std::string_view str);
        [[nodiscard]] std::string_view GetHeaderValue(std::string_view key) const;
        [[nodiscard]] std::string_view Method() const;
        [[nodiscard]] std::string_view Path() const;
        [[nodiscard]] int8_t           MinorVersion() const;

    private:
        std::string_view TrimSpace(std::string_view str);

    private:
        // HeaderField <-> HeaderValue
        using HttpHeader = std::pair<std::string_view, std::string_view>;

        constexpr static size_t MAX_HEADER_FIELD_NUM = 100;

        std::string_view                                       _method;
        std::string_view                                       _originalUrl;
        std::string_view                                       _path;
        std::unordered_map<std::string_view, std::string_view> _queries;
        std::array<HttpHeader, MAX_HEADER_FIELD_NUM>           _headers;
        int8_t                                                 _minorVersion = -1;
        int                                                    _headerLen    = 0;
        int                                                    _bodyLen      = 0;
        size_t                                                 _numHeaders   = 0;

        // response only
        int              _status = 0;
        std::string_view _msg;
    };
} // namespace Http