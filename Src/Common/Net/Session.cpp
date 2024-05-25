﻿/*************************************************************************
> File Name       : Session.cpp
> Brief           : 网络会话
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月08日  16时51分30秒
************************************************************************/
#include "Session.h"
#include "Common/Util/Log.h"

// #include "NetMessage.pb.h"

namespace Net
{
    ISession::ISession(asio::ip::tcp::socket &&socket)
        : _socket(std::move(socket))
        , _remoteAddress(_socket.remote_endpoint().address())
        , _remotePort(_socket.remote_endpoint().port())
        , _closed(false)
        , _closing(false)
    {
    }

    ISession::~ISession()
    {
        if (_closed)
        {
            return;
        }

        std::error_code                  errcode;
        [[maybe_unused]] std::error_code ret = _socket.close(errcode);
    }

    void ISession::StartSession()
    {
        asio::co_spawn(_socket.get_executor(), ReadLoop(), asio::detached);
        asio::co_spawn(_socket.get_executor(), WriteLoop(), asio::detached);
    }

    void ISession::SendProtoMessage(size_t header, const std::string &message)
    {
        // NetMessage::Message send;
        // send.set_header(static_cast<uint32_t>(header));
        // Log::Debug("header = {}", header);
        // send.set_content(message);
        // MessageBuffer content(send.ByteSizeLong());
        // if (send.SerializeToArray(content.GetWritPointer(), (int)content.WritableBytes()))
        // {
        //     content.WriteDone(send.ByteSizeLong());
        //     Log::Debug("send len:{}", content.ReadableBytes());
        //     _writeBufferQueue.push(std::move(content));

        //     AsyncWrite();
        // }
        // else
        // {
        //     Log::Error("发送消息出错：【header:{}, content:{}】", header, message);
        // }
    }

    void ISession::SendMsg(const MessageBuffer &message)
    {
        if (message.ReadableBytes() <= 0)
        {
            return;
        }

        _writeBufferQueue.Push(message);
        // AsyncWrite();
    }

    void ISession::CloseSession()
    {
        if (_closed.exchange(true))
        {
            return;
        }

        std::error_code                  error;
        [[maybe_unused]] std::error_code ret = _socket.shutdown(asio::socket_base::shutdown_send, error);
        if (error)
        {
            Log::Error("关闭网络会话错误，IP:{} errorCode:{} message:{}",
                       GetRemoteIpAddress(),
                       error.value(),
                       error.message());
        }
    }

    asio::awaitable<void> ISession::ReadLoop()
    {
        while (true)
        {
            Log::Debug("readLoop");
            auto [errcode, length] = co_await _socket.async_read_some(
                asio::buffer(_buffer.GetWritPointer(), _buffer.WritableBytes()));

            Log::Debug("after readLoop:{}", length);
            if (errcode)
            {
                if (errcode != asio::error::eof)
                {
                    Log::Error("读取消息出错：{}", errcode.message());
                }

                Log::Error("读取消息出错：{}", errcode.message());
                CloseSession();
                co_return;
            }

            _buffer.WriteDone(length);
            _readBufferQueue.Push(std::move(_buffer));
        }

        // auto self(shared_from_this());
        // _socket.async_read_some(asio::buffer(_buffer.GetWritPointer(), _buffer.WritableBytes()),
        //                         [self](const std::error_code &errcode, size_t length) mutable {
        //                             if (errcode)
        //                             {
        //                                 if (errcode != asio::error::eof)
        //                                 {
        //                                     Log::Error("读取消息出错：{}", errcode.message());
        //                                 }

        //                                 self->CloseSession();
        //                                 return;
        //                             }

        //                             // if (length == 0)
        //                             // {
        //                             //     Log::Warn("读取数据长度为0");
        //                             //     return;
        //                             // }

        //                             Log::Debug("async Read:{}", length);

        //                             // 更新写入数据量
        //                             self->_buffer.WriteDone(length);

        //                             self->_readBufferQueue.Push(std::move(self->_buffer));

        //                             self->AsyncRead();
        //                         });
    }

    asio::awaitable<void> ISession::WriteLoop()
    {
        while (true)
        {
            if (_writeBufferQueue.Empty())
            {
                continue;
            }

            MessageBuffer packet;
            if (!_writeBufferQueue.Pop(packet))
            {
                continue;
            }

            auto [errcode, length] =
                co_await asio::async_write(_socket,
                                           asio::buffer(packet.GetReadPointer(), packet.ReadableBytes()));
            if (errcode)
            {
                CloseSession();
                Log::Error("发送消息失败：{}", errcode.message());
                co_return;
            }
        }

        // MessageBuffer packet;
        // if (!_writeBufferQueue.Pop(packet))
        // {
        //     co_return;
        // }

        // auto self(shared_from_this());
        // asio::async_write(_socket,
        //                   asio::buffer(packet.GetReadPointer(), packet.ReadableBytes()),
        //                   [self](const std::error_code &errcode, std::size_t length) mutable {
        //                       if (errcode)
        //                       {
        //                           self->CloseSession();
        //                           Log::Error("发送消息失败：{}", errcode.message());
        //                           return;
        //                       }

        //                       Log::Debug("Write done:{} currend thread:{}", length);
        //                       // packet.ReadDone(length);

        //                       // if (packet.ReadableBytes() != 0)
        //                       // {
        //                       //     Log::Warn("packet的消息未读处理完就抛弃！！！");
        //                       // }

        //                       if (!self->_writeBufferQueue.Empty())
        //                       {
        //                           self->AsyncWrite();
        //                       }
        //                       else if (self->_closing)
        //                       {
        //                           self->CloseSession();
        //                       }
        //                   });
    }
} // namespace Net