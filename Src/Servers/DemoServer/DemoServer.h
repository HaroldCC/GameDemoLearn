#pragma once

#include "Common/Net/Server.h"

class DemoServer : public Net::IServer
{
public:
    using Net::IServer::IServer;

protected:
    void OnScoketAccepted(Asio::socket &&socket) override;

private:
};