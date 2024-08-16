/*************************************************************************
> File Name       : HttpSession.h
> Brief           : Http会话
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月09日  16时34分08秒
************************************************************************/
#pragma once
#include "Common/Net/Session.h"
#include "Common/Net/Http/HttpRequest.h"
#include "Common/Net/Http/HttpResponse.h"
#include "Common/Net/Http/HttpRouter.h"

namespace Http
{
    class HttpSession final : public Net::ISession
    {
    public:
        using Net::ISession::ISession;
        // HttpSession(Asio::socket &&socket, asio::io_context *pLogicIOCtx, HttpRouter &router);
        void SetRouter(HttpRouter &router);

    protected:
        void                  ReadHandler() override;
        asio::awaitable<void> CoReadHandler();

    private:
        HttpRequest  _req;
        HttpResponse _rep;
        HttpRouter   _router;
    };

} // namespace Http