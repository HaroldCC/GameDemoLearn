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
        friend class IServer;

    public:
        ISession(const ISession &)            = delete;
        ISession(ISession &&)                 = delete;
        ISession &operator=(const ISession &) = delete;
        ISession &operator=(ISession &&)      = delete;

        explicit ISession(asio::ip::tcp::socket &&socket);

        virtual ~ISession();

        void StartSession();

        void SendProtoMessage(size_t header, const std::string &message);

        void SendMsg(const MessageBuffer &message);

        void CloseSession();

        std::string GetRemoteIpAddress()
        {
            return _remoteAddress.to_string();
        }

        bool IsAlive()
        {
            return !_closed && !_closing;
        }

        void DelayCloseSession()
        {
            _closing = true;
        }

    protected:
        virtual bool Update()
        {
            ReadHandler();
            return !_closed;
        }

        virtual void ReadHandler() = 0;

        template <typename Proto>
        std::optional<Proto> BufferToProto(MessageBuffer &buffer)
        {
            Proto proto;
            if (!proto.ParseFromArray(buffer.GetReadPointer(), static_cast<int>(buffer.ReadableBytes())))
            {
                return std::nullopt;
            }

            buffer.ReadDone(buffer.ReadableBytes());

            return proto;
        }

        template <typename Proto>
        std::optional<MessageBuffer> ProtoToBuffer(const Proto &proto)
        {
            auto          size = proto.ByteSizeLong();
            MessageBuffer message(size);
            if (!proto.SerializeToArray(message.GetWritPointer(), (int)size))
            {
                return std::nullopt;
            }
            message.WriteDone(size);

            return message;
        }

        asio::awaitable<void> ReadLoop();

        asio::awaitable<void> WriteLoop();

    protected:
        Asio::socket                         _socket;
        Asio::address                        _remoteAddress;
        Asio::steady_timer                   _timer;
        uint16_t                             _remotePort;
        uint32_t                             _header {0};
        ProducerConsumerQueue<MessageBuffer> _readBufferQueue;
        ProducerConsumerQueue<MessageBuffer> _writeBufferQueue;

        std::atomic_bool _closed;
        std::atomic_bool _closing;
    };
} // namespace Net