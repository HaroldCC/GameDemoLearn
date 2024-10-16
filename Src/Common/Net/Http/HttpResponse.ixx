/*************************************************************************
> File Name       : HttpResponse.h
> Brief           : Http响应
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月09日  15时36分21秒
************************************************************************/

module;

#include <vector>
#include <string_view>
#include <string>
#include <format>

export module Common:Net.HttpResponse;

import :Net.HttpCommon;
import :Net.HttpUtil;
import :Util;
import :TimeUtil;

export namespace Http
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

// module :private;

using namespace Http;

/**
     * @brief 获取要发送的Http响应内容
     *
     * @return std::string_view 响应内容
     */
[[nodiscard]] std::string_view HttpResponse::GetPayload()
{
    BuildResponseHead();
    _head.append(std::format("{}\r\n{}", Http::ContentTypeString(_contentType, _charset), _content));
    return _head;
}

void HttpResponse::SetStatusCode(StatusCode statusCode)
{
    _statusCode = statusCode;
}

void HttpResponse::SetHeader(std::string_view fieldName, std::string_view fieldVal)
{
    _headers.emplace_back(fieldName, fieldVal);
}

void HttpResponse::SetCharSet(std::string_view charset /*= "UTF-8"*/)
{
    _charset = charset;
}

void HttpResponse::SetContentType(ContentType type)
{
    _contentType = type;
}

void HttpResponse::SetContent(std::string_view content)
{
    _content = content;
}

void HttpResponse::FillResponse(StatusCode statusCode, ContentType type, std::string_view content)
{
    _statusCode  = statusCode;
    _contentType = type;
    _content     = content;
}

void HttpResponse::BuildResponseHead()
{
    if (std::find_if(_headers.begin(),
                     _headers.end(),
                     [](ResponseHeader &header) {
                         return header.first == "Host";
                     })
        == _headers.end())
    {
        _headers.emplace_back("Host", "Harold");
    }

    if (_statusCode >= StatusCode::NotFound)
    {
        _content.append(StatusToResponseContent(_statusCode));
    }

    if (_content.empty())
    {
        _headers.emplace_back("Content-Length", "0");
    }
    else
    {
        _headers.emplace_back("Content-Length", Util::ToString(_content.size()));
    }

    _headers.emplace_back("Date", TimeUtil::GetGMTTimeStr());

    _head.append(StatusToResponseHead(_statusCode));
    for (auto &[k, v] : _headers)
    {
        _head.append(std::format("{}:{}\r\n", k, v));
    }
}
