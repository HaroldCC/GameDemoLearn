/*************************************************************************
> File Name       : LoginSession.h
> Brief           : 登录会话
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年10月14日  18时00分01秒
************************************************************************/
#pragma once

#include "Common/Net/Session.h"

class LoginSession final : public Net::ISession
{
public:
protected:
    void OnMessageReceived(Net::MessageBuffer &buffer) override;
};
