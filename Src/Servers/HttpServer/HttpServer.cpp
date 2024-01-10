/*************************************************************************
> File Name       : HttpServer.cpp
> Brief           : Http服务器
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月10日  15时06分14秒
************************************************************************/
#include "HttpServer.h"
#include "Common/Net/Http/HttpCommon.h"
#include "Common/Util/Log.h"
#include "Common/Util/Assert.h"

void HttpServer::InitHttpRouter(const std::shared_ptr<Http::HttpSession> &pSession)
{
    Assert(nullptr != pSession);

    pSession->AddRouter(Http::HttpMethod::Get,
                        "/",
                        [](const Http::HttpRequest &request, Http::HttpResponse &resp) {
                            resp.SetStatusCode(Http::StatusCode::Ok);
                            std::string_view body = "Hello 你好!";
                            resp.SetCharSet("UTF-8BOM");
                            resp.SetContent(body);
                            return;
                        });
}

void HttpServer::DoAccept()
{
    _acceptor.async_accept([this](const std::error_code &errcode, asio::ip::tcp::socket socket) {
        if (errcode)
        {
            Log::Error("接受连接失败：{}", errcode.message());
            return;
        }

        auto pSession = std::make_shared<Http::HttpSession>(std::move(socket));
        _sessions.emplace_back(pSession);
        InitHttpRouter(pSession);
        pSession->StartSession();

        DoAccept();
    });
}