﻿/*************************************************************************
> File Name       : HttpServer.h
> Brief           : Http服务器
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月10日  15时02分44秒
************************************************************************/
#pragma once

#include "Common/Net/Server.h"
#include "HttpSession.h"

class HttpServer final : public Net::IServer
{
public:
    using Net::IServer::IServer;

    void InitHttpRouter(const std::shared_ptr<Http::HttpSession> &pSession);

protected:
    void DoAccept() override;
};