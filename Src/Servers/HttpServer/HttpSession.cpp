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

    void HttpSession::AddRouter(HttpMethod method, std::string_view path, HttpHandlerFunc handler)
    {
        _router.AddHttpHandler(method, path, std::move(handler));
    }

    void HttpSession::ReadHandler()
    {
        Net::MessageBuffer packet;
        if (!_readBufferQueue.Pop(packet))
        {
            return;
        }

        if (packet.ReadableBytes() <= 0)
        {
            return;
        }

        const std::string &content = packet.ReadAllAsString();
        if (!content.empty())
        {
            // Log::Info("Http:{}", content);
            _req.Parse(content);
            _router.Route(_req, _rep);

            std::string_view   response = _rep.GetPayload();
            Net::MessageBuffer sendBuffer(response.size());
            sendBuffer.Write(response);
            SendMsg(std::move(sendBuffer));
            // SendProtoMessage((uint32_t)response.size(), response.data());
        }
        else
        {
            Log::Error("解析Http请求内容出错");
        }
    }
} // namespace Http