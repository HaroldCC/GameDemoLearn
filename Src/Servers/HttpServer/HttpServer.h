/*************************************************************************
> File Name       : HttpServer.h
> Brief           : Http服务器
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月10日  15时02分44秒
************************************************************************/
#pragma once

#include "HttpSession.h"
#include "Common/Net/Server.h"
#include "Common/Database/QueryCallback.h"

class HttpServer final : public Net::IServer
{
public:
    HttpServer(std::string_view ip, uint16_t port);

    void InitHttpRouter();

protected:
    void Update() override;

    void OnScoketAccepted(Asio::socket &&socket) override;

private:
    Database::QueryCallbackProcessor _queryCallbackProcessor;
    Http::HttpRouter                 _router;
};