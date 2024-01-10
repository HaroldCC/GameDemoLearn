/*************************************************************************
> File Name       : HttpResponse.h
> Brief           : Http响应
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月09日  15时36分21秒
************************************************************************/
#pragma once
#include "HttpCommon.h"

#include <vector>
#include <string_view>

namespace Http
{
    class HttpResponse final
    {
    public:
        /**
         * @brief 获取要发送的Http响应内容
         *
         * @return std::string_view 响应内容
         */
        [[nodiscard]] std::string_view GetPayload();

        void SetStatusCode(StatusCode statusCode);
        void SetHeader(std::string_view fieldName, std::string_view fieldVal);
        void SetCharSet(std::string_view charset = "UTF-8");
        void SetContentType(ContentType type);
        void SetContent(std::string_view content);

        void FillResponse(StatusCode statusCode, ContentType type, std::string_view content);

    private:
        void BuildResponseHead();

    private:
        using ResponseHeader = std::pair<std::string, std::string>;

        StatusCode                  _statusCode  = StatusCode::Unknown;
        ContentType                 _contentType = ContentType::String;
        std::string                 _head;
        std::string                 _content;
        std::string_view            _charset {"UTF-8"};
        std::vector<ResponseHeader> _headers;
    };
} // namespace Http