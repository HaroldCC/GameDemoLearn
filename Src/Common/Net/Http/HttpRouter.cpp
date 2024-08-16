/*************************************************************************
> File Name       : HttpRouter.cpp
> Brief           : Http路由
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月10日  14时35分01秒
************************************************************************/
#include "HttpRouter.h"
#include "HttpUtil.h"
#include "Common/Util/Log.h"
#include "HttpCommon.h"

#include <format>

namespace Http
{

    asio::awaitable<void> HttpRouter::Route(const HttpRequest &req, HttpResponse &resp)
    {
        std::string methodWholeName = std::format("{} {}", req.GetMethod(), req.GetPath());

        if (auto iter = _httpHandlers.find(methodWholeName); iter != _httpHandlers.end())
        {
            try
            {
                co_await iter->second(req, resp);
            }
            catch (const std::exception &e)
            {
                Log::Critical("Http方法抛出异常, reason:{}", e.what());
                resp.SetStatusCode(StatusCode::ServiceUnavailable);
            }
            catch (...)
            {
                Log::Critical("Http抛出未知异常!!!");
                resp.SetStatusCode(StatusCode::ServiceUnavailable);
            }
        }
        else
        {
            resp.SetStatusCode(StatusCode::NotFound);
        }
    }

    /**
     * @brief 添加Http处理函数  eg: GET hello/    POST hello/ 作为key
     * 
     * @param method 方法
     * @param path 路径
     * @param handler 处理函数
     */
    void HttpRouter::AddHttpHandler(HttpMethod method, std::string_view path, HttpHandlerFunc handler)
    {
        std::string_view methodName = HttpMethodToString(method);
        std::string      wholeStr   = std::format("{} {}", methodName, path);

        auto [it, ok] = _pathKeys.emplace(std::move(wholeStr));
        if (!ok)
        {
            Log::Critical("http {} 已经注册");
            return;
        }

        _httpHandlers.emplace(*it, std::move(handler));
    }
} // namespace Http