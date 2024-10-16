/*************************************************************************
> File Name       : LoginServer.h
> Brief           : 登陆服
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年10月14日  10时50分54秒
************************************************************************/

export module LoginServer;

import Common;
import LoginSession;

class LoginServer final : public Net::IServer
{
public:
    LoginServer(std::string_view ip, uint16_t port);

protected:
    void                           Update() override;
    std::shared_ptr<Net::ISession> CreateSession(Asio::socket &&socket) override;

private:
};

// module :private;

LoginServer::LoginServer(std::string_view ip, uint16_t port) : Net::IServer(ip, port)
{
}

void LoginServer::Update()
{
    Net::IServer::Update();
}

std::shared_ptr<Net::ISession> LoginServer::CreateSession(Asio::socket &&socket)
{
    return std::make_shared<LoginSession>(std::move(socket));
}
