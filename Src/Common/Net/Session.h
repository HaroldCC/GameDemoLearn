/*************************************************************************
> File Name       : Session.h
> Brief           : 网络会话
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月08日  16时24分41秒
************************************************************************/
#pragma once

#include "Common/Util/Platform.h"
#include "Buffer.h"
#include "Asio.h"
#include "Common/Util/ProducerConsumerQueue.hpp"

namespace Net
{
    class ISession : public std::enable_shared_from_this<ISession>
    {
    public:
        ISession(const ISession &)            = delete;
        ISession(ISession &&)                 = delete;
        ISession &operator=(const ISession &) = delete;
        ISession &operator=(ISession &&)      = delete;

        explicit ISession(asio::ip::tcp::socket &&socket);
        virtual ~ISession();

        void StartSession();
        void CloseSession();
        bool Update();

        void SendMessage(const MessageBuffer &message);
        
        std::string GetRemoteIpAddress() const { return _remoteAddress.to_string(); }
        uint16_t GetRemotePort() const { return _remotePort; }
        bool IsAlive() const { return !_closed && !_closing; }
        void DelayCloseSession() { _closing = true; }

    protected:
        virtual void OnMessageReceived(MessageBuffer& buffer) = 0;

    private:
        asio::awaitable<void> ReadLoop();
        asio::awaitable<void> WriteLoop();

    protected:
        Asio::socket _socket;
        Asio::address _remoteAddress;
        Asio::steady_timer _timer;
        uint16_t _remotePort;
        ProducerConsumerQueue<MessageBuffer> _readBufferQueue;
        ProducerConsumerQueue<MessageBuffer> _writeBufferQueue;

        std::atomic_bool _closed;
        std::atomic_bool _closing;
    };
} // namespace Net
