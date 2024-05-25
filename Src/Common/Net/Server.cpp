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
    IServer::IServer(std::string_view ip, uint16_t port)
        : _ioCtx(1)
        , _logicCtx(1)
        , _signals(_ioCtx)
        , _listenEndPoint(Asio::make_address(ip), port)
        , _acceptor(_ioCtx, _listenEndPoint)
        , _updateTimer(_logicCtx)
    {
    }

    IServer::~IServer()
    {
        Log::Debug("~IServer");
        if (_logicThread.joinable())
        {
            _logicThread.join();
        }
    }

    void IServer::Start()
    {
        // 启动逻辑线程
        auto self(shared_from_this());
        _logicThread = std::thread([self]() {
            try
            {
                using namespace std::chrono_literals;
                self->_updateTimer.expires_from_now(1ms);
                self->_updateTimer.async_wait([self](const std::error_code &errcode) {
                    self->Update();
                });
                std::stringstream ss;
                ss << std::this_thread::get_id();
                Log::Debug("逻辑线程启动：{}", ss.str());
                self->_logicCtx.run();
            }
            catch (const std::exception &e)
            {
                Log::Error("服务器异常退出：{}", e.what());
            }
            catch (...)
            {
                Log::Critical("服务器未知异常！！！");
            }
        });

        // 启动网络协程
        try
        {
            _signals.add(SIGINT);
            _signals.add(SIGTERM);
            _signals.async_wait([this](const std::error_code &errcode, int signal) {
                _ioCtx.stop();
            });

            Asio::co_spawn(_ioCtx, AcceptLoop(), asio::detached);
            _ioCtx.run();
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

    void IServer::Update()
    {
        using namespace std::chrono_literals;
        _updateTimer.expires_from_now(1ms);
        _updateTimer.async_wait([this](const std::error_code &errcode) {
            Update();
        });

        if (_sessions.empty())
        {
            return;
        }

        std::lock_guard lock(_mutex);
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