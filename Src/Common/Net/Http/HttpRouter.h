/*************************************************************************
> File Name       : HttpRouter.h
> Brief           : Http路由
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月09日  16时36分43秒
************************************************************************/
#pragma once

#include "HttpRequest.h"
#include "HttpResponse.h"

#include <functional>
#include <unordered_map>
#include <set>

#include "asio.hpp"

namespace Http
{
    using HttpHandlerFunc = std::function<asio::awaitable<void>(const HttpRequest &, HttpResponse &)>;

    class HttpRouter final
    {
    public:
        void Route(const HttpRequest &req, HttpResponse &resp);

        /**
         * @brief 添加Http处理函数  eg: GET hello/    POST hello/ 作为key
         * 
         * @param method 方法
         * @param path 路径
         * @param handler 处理函数
         */
        void AddHttpHandler(HttpMethod method, std::string_view path, HttpHandlerFunc handler);

    private:
        std::unordered_map<std::string_view, HttpHandlerFunc> _httpHandlers;
        std::set<std::string>                                 _pathKeys;
    };
} // namespace Http