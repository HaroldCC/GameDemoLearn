/*************************************************************************
> File Name       : HttpServer.cpp
> Brief           : Http服务器
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月10日  15时06分14秒
************************************************************************/
#include "HttpServer.h"
#include "Common/Net/Http/HttpCommon.h"
#include "Common/Util/Log.h"
#include "Common/Util/Assert.h"
#include "Common/Database/DatabaseImpl/LoginDatabase.h"

void HttpServer::InitHttpRouter(const std::shared_ptr<Http::HttpSession> &pSession)
{
    Assert(nullptr != pSession);

    pSession->AddRouter(Http::HttpMethod::Get,
                        "/",
                        [](const Http::HttpRequest &request, Http::HttpResponse &resp) {
                            resp.SetStatusCode(Http::StatusCode::Ok);
                            std::string_view body = "Hello 你好!";
                            resp.SetCharSet("UTF-8BOM");
                            resp.SetContent(body);
                            return;
                        });
    pSession->AddRouter(
        Http::HttpMethod::Get,
        "/user",
        [this](const Http::HttpRequest &request, Http::HttpResponse &resp) {
            auto data =
                Database::g_LoginDatabaseStmts.find(Database::LoginDatabaseSqlID::LOGIN_SEL_ACCOUNT_BY_EMAIL);
            if (data == Database::g_LoginDatabaseStmts.end())
            {
                resp.SetStatusCode(Http::StatusCode::InternalServerError);
                return;
            }

            Database::PreparedStatementBase *pStmt = Database::g_LoginDatabase.GetPrepareStatement(
                Database::LoginDatabaseSqlID::LOGIN_SEL_ACCOUNT_BY_EMAIL);
            pStmt->SerialValue(data->second, "123456@qq.com");

            //Database::PreparedQueryResultSetPtr pResult = Database::g_LoginDatabase.SyncQuery(pStmt);
            _queryCallbackProcessor.AddCallback(Database::g_LoginDatabase.AsyncQuery(pStmt).Then(
                [&resp](Database::PreparedQueryResultSetPtr pResult) {
                    if (nullptr == pResult)
                    {
                        resp.SetStatusCode(Http::StatusCode::InternalServerError);
                        return;
                    }

                    Database::Field *pFields = pResult->Fetch();
                    uint32_t         id      = pFields[0];
                    const char      *strName = pFields[1];
                    const char      *email   = pFields[2];
                    uint32_t         age     = pFields[3];
                    std::string      intro   = pFields[4];

                    resp.SetContent(std::format("id:{}, email:{}, name:{}, age:{}, intro:{}",
                                                id,
                                                email,
                                                strName,
                                                age,
                                                intro));
                    Log::Debug("id:{}, email:{}, name:{}, age:{}, intro:{}", id, email, strName, age, intro);

                    resp.SetStatusCode(Http::StatusCode::Ok);
                }));

            std::this_thread::sleep_for(std::chrono::seconds(5));
        });
}

void HttpServer::DoAccept()
{
    _acceptor.async_accept([this](const std::error_code &errcode, asio::ip::tcp::socket socket) {
        if (errcode)
        {
            Log::Error("接受连接失败：{}", errcode.message());
            return;
        }

        auto pSession = std::make_shared<Http::HttpSession>(std::move(socket));
        _sessions.emplace_back(pSession);
        InitHttpRouter(pSession);
        pSession->StartSession();

        DoAccept();
    });
}

void HttpServer::Update()
{
    _queryCallbackProcessor.ProcessReadyCallbacks();
}