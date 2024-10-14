/*************************************************************************
> File Name       : HttpSession.cpp
> Brief           : Http会话
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月10日  14时53分05秒
************************************************************************/
#include "HttpSession.h"
#include "Common/Util/Log.h"

namespace Http
{
    HttpSession::HttpSession(Asio::socket&& socket)
        : Net::ISession(std::move(socket))
    {
    }

    void HttpSession::SetRouter(const HttpRouter& router)
    {
        _router = router;
    }

    void HttpSession::OnMessageReceived(Net::MessageBuffer& buffer)
    {
        const std::string &content = buffer.ReadAllAsString();
        if (!content.empty())
        {
            _req.Parse(content);
            _router.Route(_req, _rep);

            std::string_view response = _rep.GetPayload();
            Net::MessageBuffer sendBuffer(response.size());
            sendBuffer.Write(response);
            SendMessage(std::move(sendBuffer));
        }
        else
        {
            Log::Error("解析Http请求内容出错");
        }
    }
} // namespace Http
