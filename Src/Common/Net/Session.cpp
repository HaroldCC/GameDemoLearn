/*************************************************************************
> File Name       : Session.cpp
> Brief           : 网络会话
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月08日  16时51分30秒
************************************************************************/
#include "Session.h"
#include "Common/Util/Log.h"

namespace Net
{
    ISession::ISession(asio::ip::tcp::socket &&socket)
        : _socket(std::move(socket))
        , _remoteAddress(socket.remote_endpoint().address())
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
        AsyncRead();
    }

    void ISession::SendProtoMessage(size_t header, const std::string &message)
    {
    }

    void ISession::SendMsg(const MessageBuffer &message)
    {
        if (message.ReadableBytes() <= 0)
        {
            return;
        }

        _writeBufferQueue.push(message);
        AsyncWrite();
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

    void ISession::AsyncRead()
    {
        auto self(shared_from_this());
        _readBuffer.EnsureFreeSpace();
        _socket.async_read_some(asio::buffer(_readBuffer.GetWritPointer(), _readBuffer.WritableBytes()),
                                [this, self](const std::error_code &errcode, size_t length) {
                                    if (errcode)
                                    {
                                        if (errcode != asio::error::eof)
                                        {
                                            Log::Error("读取消息出错：{}", errcode.message());
                                        }

                                        CloseSession();
                                        return;
                                    }

                                    // 更新写入数据量
                                    _readBuffer.WriteDone(length);

                                    ReadHandler();

                                    AsyncRead();
                                });
    }

    void ISession::AsyncWrite()
    {
        if (_writeBufferQueue.empty())
        {
            return;
        }

        auto           self(shared_from_this());
        MessageBuffer &packet = _writeBufferQueue.front();
        _socket.async_write_some(asio::buffer(packet.GetReadPointer(), packet.ReadableBytes()),
                                 [this, self](const std::error_code &errcode, std::size_t length) {
                                     if (errcode)
                                     {
                                         CloseSession();
                                         Log::Error("发送消息失败：{}", errcode.message());
                                         return;
                                     }

                                     // 更新从buff读出来多少数据
                                     auto &&buff = _writeBufferQueue.front();
                                     buff.ReadDone(length);

                                     if (buff.ReadableBytes() == 0)
                                     {
                                         _writeBufferQueue.pop();
                                     }

                                     if (!_writeBufferQueue.empty())
                                     {
                                         AsyncWrite();
                                     }
                                     else if (_closing)
                                     {
                                         CloseSession();
                                     }
                                 });
    }
} // namespace Net