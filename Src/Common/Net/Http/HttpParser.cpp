/*************************************************************************
> File Name       : HttpParser.cpp
> Brief           : Http解析
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月09日  14时44分45秒
************************************************************************/
#include "HttpParser.h"
#include "picohttpparser.h"
#include "Common/Util/Log.h"
#include "Common/Util/Util.h"

using namespace std::string_view_literals;

namespace Http
{
    size_t HttpParser::ParseRequest(std::string_view originalUrl)
    {
        if (originalUrl.empty())
        {
            return 0;
        }

        const char                                  *method       = nullptr;
        size_t                                       methodLen    = 0;
        const char                                  *url          = nullptr;
        size_t                                       urlLen       = 0;
        int                                          minorVersion = -1;
        std::array<phr_header, MAX_HEADER_FIELD_NUM> headers {};
        _numHeaders = MAX_HEADER_FIELD_NUM;

        _headerLen = phr_parse_request(originalUrl.data(),
                                       originalUrl.size(),
                                       &method,
                                       &methodLen,
                                       &url,
                                       &urlLen,
                                       &minorVersion,
                                       headers.data(),
                                       &_numHeaders,
                                       0);
        if (_headerLen < 0)
        {
            Log::Error("Parse http request failed");
            return 0;
        }

        _method       = {method, methodLen};
        _path         = {url, urlLen};
        _minorVersion = static_cast<int8_t>(minorVersion);
        _originalUrl  = originalUrl;

        auto toHttpHeader = [](const phr_header &header) -> HttpHeader {
            return {{header.name, header.name_len}, {header.value, header.value_len}};
        };

        for (size_t i = 0; i < headers.size(); ++i)
        {
            _headers[i] = toHttpHeader(headers[i]);
        }

        std::string_view contentLen = GetHeaderValue(("content-length"sv));
        _bodyLen                    = Util::StringTo<int>(contentLen).value_or(0);

        size_t pos = _path.find('?');
        if (pos != std::string_view::npos)
        {
            ParseQuery(_path.substr(pos + 1, urlLen - pos - 1));
            _path = {url, pos};
        }

        return _headerLen;
    }

    size_t HttpParser::ParseResponse(std::string_view originalUrl)
    {
        if (originalUrl.empty())
        {
            return 0;
        }

        int         minorVersion = 0;
        const char *msg          = nullptr;
        size_t      msgLen       = 0;
        _numHeaders              = MAX_HEADER_FIELD_NUM;
        std::array<phr_header, MAX_HEADER_FIELD_NUM> headers {};
        _headerLen                  = phr_parse_response(originalUrl.data(),
                                        originalUrl.size(),
                                        &minorVersion,
                                        &_status,
                                        &msg,
                                        &msgLen,
                                        headers.data(),
                                        &_numHeaders,
                                        0);
        _msg                        = {msg, msgLen};
        std::string_view contentLen = GetHeaderValue("content-length"sv);
        _bodyLen                    = Util::StringTo<int>(contentLen).value_or(0);
        if (_headerLen < 0)
        {
            Log::Error("Parse http response failed");
            return 0;
        }

        _minorVersion = static_cast<int8_t>(minorVersion);

        return _headerLen;
    }

    void HttpParser::ParseQuery(std::string_view str)
    {
        if (str.empty())
        {
            return;
        }

        std::string_view key;
        std::string_view val;
        size_t           length = str.length();
        size_t           pos    = 0;
        for (size_t i = 0; i < length; ++i)
        {
            char chr = str[i];
            if (chr == '=')
            {
                key = {&str[pos], i - pos};
                key = TrimSpace(key);
                pos = i + 1;
            }
            else if (chr == '&')
            {
                val = {&str[pos], i - pos};
                val = TrimSpace(val);
                _queries.emplace(key, val);

                pos = i + 1;
            }
        }

        if (pos == 0)
        {
            return;
        }

        if ((length - pos) > 0)
        {
            val = {&str[pos], length - pos};
            val = TrimSpace(val);
            _queries.try_emplace(key, val);
        }
        else if ((length - pos) == 0)
        {
            _queries.emplace(key, "");
        }
    }

    [[nodiscard]] std::string_view HttpParser::GetHeaderValue(std::string_view key) const
    {
        for (auto &&header : _headers)
        {
            if (Util::StringEqual(header.first, key))
            {
                return header.second;
            }
        }

        return {};
    }

    [[nodiscard]] std::string_view HttpParser::Method() const
    {
        return _method;
    }

    [[nodiscard]] std::string_view HttpParser::Path() const
    {
        return _path;
    }

    [[nodiscard]] int8_t HttpParser::MinorVersion() const
    {
        return _minorVersion;
    }

    std::string_view HttpParser::TrimSpace(std::string_view str)
    {
        str.remove_prefix(std::min(str.find_first_not_of(' '), str.size()));
        str.remove_suffix(std::min(str.size() - str.find_last_not_of(' ') - 1, str.size()));

        return str;
    }
} // namespace Http