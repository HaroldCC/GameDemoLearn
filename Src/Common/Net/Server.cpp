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
        : _netIoCtx(1)
        , _logicIoCtx(1)
        , _signals(_netIoCtx)
        , _listenEndPoint(Asio::make_address(ip), port)
        , _acceptor(_netIoCtx, _listenEndPoint)
        , _updateTimer(_logicIoCtx)
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
        // auto self(shared_from_this());
        _logicThread = std::thread([this]() {
            try
            {
                using namespace std::chrono_literals;
                _updateTimer.expires_from_now(1ms);
                _updateTimer.async_wait([this](const std::error_code &errcode) {
                    Update();
                });
                std::stringstream ss;
                ss << std::this_thread::get_id();
                Log::Debug("逻辑线程启动：{}", ss.str());
                // auto wg =
                //     asio::require(_logicIoCtx.get_executor(), asio::execution::outstanding_work.tracked);
                _logicIoCtx.run();
                Log::Debug("logic thread stoping.....");
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
            // _signals.add(SIGINT);
            _signals.add(SIGTERM);
            _signals.async_wait([this](const std::error_code &errcode, int signal) {
                Log::Debug("signal:{}", signal);
                _netIoCtx.stop();
            });

            Asio::co_spawn(_netIoCtx, AcceptLoop(), asio::detached);
            _netIoCtx.run();
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

    asio::awaitable<void> IServer::AcceptLoop()
    {
        while (true)
        {
            auto [errcode, socket] = co_await _acceptor.async_accept();
            if (errcode)
            {
                Log::Error("接受连接失败：{}", errcode.message());
                co_return;
            }

            OnScoketAccepted(std::move(socket), &_logicIoCtx);
        }
    }

    void IServer::AddNewSession(std::shared_ptr<ISession> pNewSession)
    {
        std::lock_guard lock(_mutex);
        _sessions.emplace_back(pNewSession);
    }
} // namespace Net