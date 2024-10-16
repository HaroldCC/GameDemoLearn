/*************************************************************************
> File Name       : HttpRouter.h
> Brief           : Http路由
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月09日  16时36分43秒
************************************************************************/
module;

#include <functional>
#include <unordered_map>
#include <set>

export module Common:Net.HttpRouter;

import :Net.HttpRequest;
import :Net.HttpResponse;
import :Net.HttpCommon;

export namespace Http
{
    using HttpHandlerFunc = std::function<void(const HttpRequest &, HttpResponse &)>;

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

module :private;

using namespace Http;

void HttpRouter::Route(const HttpRequest &req, HttpResponse &resp)
{
    std::string methodWholeName = std::format("{} {}", req.GetMethod(), req.GetPath());

    if (auto iter = _httpHandlers.find(methodWholeName); iter != _httpHandlers.end())
    {
        try
        {
            iter->second(req, resp);
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