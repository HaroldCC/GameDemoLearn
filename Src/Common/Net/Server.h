/*************************************************************************
> File Name       : Server.h
> Brief           : 服务器
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月08日  17时12分04秒
************************************************************************/
#pragma once
#include "Asio.h"

namespace Net
{

    class ISession;

    class IServer //: public std::enable_shared_from_this<IServer>
    {
    public:
        IServer(const IServer &)            = delete;
        IServer(IServer &&)                 = delete;
        IServer &operator=(const IServer &) = delete;
        IServer &operator=(IServer &&)      = delete;

        IServer(std::string_view ip, uint16_t port);

        virtual ~IServer();

        void Start();

    protected:
        virtual void Update();

        asio::awaitable<void> AcceptLoop();

        virtual void AddNewSession(std::shared_ptr<ISession> pNewSession);

        virtual void OnScoketAccepted(Asio::socket &&socket) = 0;

    protected:
        std::thread                                  _logicThread;
        std::mutex                                   _mutex;
        Asio::io_context                             _netIoCtx;
        Asio::io_context                             _logicIoCtx;
        Asio::signal_set                             _signals;
        Asio::endpoint                               _listenEndPoint;
        Asio::acceptor                               _acceptor;
        Asio::steady_timer                           _updateTimer;
        std::vector<std::shared_ptr<class ISession>> _sessions;
    };
} // namespace Net