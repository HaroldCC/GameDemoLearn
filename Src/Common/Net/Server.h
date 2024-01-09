/*************************************************************************
> File Name       : Server.h
> Brief           : 服务器
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月08日  17时12分04秒
************************************************************************/
#pragma once
#include "Common/Util/Platform.h"
#include "asio.hpp"

namespace Net
{
    class IServer
    {
    public:
        IServer(const IServer &)            = delete;
        IServer(IServer &&)                 = delete;
        IServer &operator=(const IServer &) = delete;
        IServer &operator=(IServer &&)      = delete;

        IServer(asio::io_context &ioCtx, uint16_t port);

        virtual ~IServer() = default;

        void Start();

    protected:
        virtual void Update();

        virtual void DoAccept() = 0;

    protected:
        std::thread                                  _netThread;
        asio::io_context                            &_ioCtx;
        asio::ip::tcp::acceptor                      _acceptor;
        asio::steady_timer                           _updateTimer;
        std::vector<std::shared_ptr<class ISession>> _sessions;
    };
} // namespace Net