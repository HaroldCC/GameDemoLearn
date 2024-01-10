/*************************************************************************
> File Name       : Server.cpp
> Brief           : 服务器
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月08日  17时43分47秒
************************************************************************/
#include "Server.h"
#include "Common/Util/Log.h"
#include "Session.h"

namespace Net
{
    IServer::IServer(asio::io_context &ioCtx, uint16_t port)
        : _ioCtx(ioCtx)
        , _acceptor(ioCtx, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
        , _updateTimer(_ioCtx)
    {
    }

    IServer::~IServer()
    {
        if (_netThread.joinable())
        {
            _netThread.join();
        }
    }

    void IServer::Start()
    {
        DoAccept();

        _updateTimer.async_wait([this](const std::error_code &errcode) {
            Update();
        });

        _netThread = std::thread([this]() {
            while (true)
            {
                try
                {
                    _ioCtx.run();
                    break;
                }
                catch (const std::exception &e)
                {
                    Log::Error("服务器异常退出：{}", e.what());
                }
                catch (...)
                {
                    Log::Critical("服务器未知异常！！！");
                }
            }
        });
    }

    void IServer::Update()
    {
        using namespace std::chrono_literals;
        _updateTimer.expires_from_now(1ms);
        _updateTimer.async_wait([this](const std::error_code &errcode) {
            Update();
        });

        _sessions.erase(std::remove_if(_sessions.begin(),
                                       _sessions.end(),
                                       [](const std::shared_ptr<ISession> &pSession) {
                                           if (!pSession->Update())
                                           {
                                               pSession->CloseSession();
                                               return true;
                                           }
                                           return false;
                                       }),
                        _sessions.end());
    }
} // namespace Net