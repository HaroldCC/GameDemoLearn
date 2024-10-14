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
#include "Session.h"

namespace Net
{
    class IServer
    {
    public:
        IServer(const IServer &)            = delete;
        IServer(IServer &&)                 = delete;
        IServer &operator=(const IServer &) = delete;
        IServer &operator=(IServer &&)      = delete;

        IServer(std::string_view ip, uint16_t port);

        virtual ~IServer();

        void Start();
        virtual void Stop();

    protected:
        virtual void Update();
        virtual std::shared_ptr<ISession> CreateSession(Asio::socket&& socket) = 0;
        
        asio::awaitable<void> AcceptLoop();
        void AddNewSession(std::shared_ptr<ISession> pNewSession);
        void RemoveSession(std::shared_ptr<ISession> pSession);

        virtual void OnSessionCreated(std::shared_ptr<ISession> pSession) {}

    protected:
        std::thread                                  _netThread;
        std::mutex                                   _mutex;
        Asio::io_context                             _netIoCtx;
        Asio::io_context                             _logicIoCtx;
        Asio::signal_set                             _signals;
        Asio::endpoint                               _listenEndPoint;
        Asio::acceptor                               _acceptor;
        Asio::steady_timer                           _updateTimer;
        std::vector<std::shared_ptr<ISession>>      _sessions;
    };
} // namespace Net
