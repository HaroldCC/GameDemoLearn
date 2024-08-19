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

HttpServer::HttpServer(std::string_view ip, uint16_t port) : Net::IServer(ip, port)
{
    InitHttpRouter();
}

void HttpServer::InitHttpRouter()
{
    _router.AddHttpHandler(
        Http::HttpMethod::Get,
        "/",
        [](const Http::HttpRequest &request, Http::HttpResponse &resp) -> asio::awaitable<void> {
            resp.SetStatusCode(Http::StatusCode::Ok);
            std::string_view body = "Hello 你好!";
            resp.SetCharSet("UTF-8BOM");
            resp.SetContent(body);
            co_return;
        });
    _router.AddHttpHandler(
        Http::HttpMethod::Get,
        "/user1",
        [this](const Http::HttpRequest &request, Http::HttpResponse &resp) -> asio::awaitable<void> {
            auto data =
                Database::g_LoginDatabaseStmts.find(Database::LoginDatabaseSqlID::LOGIN_SEL_ACCOUNT_BY_EMAIL);
            if (data == Database::g_LoginDatabaseStmts.end())
            {
                resp.SetStatusCode(Http::StatusCode::InternalServerError);
                co_return;
            }
            Log::Debug("/user request");
            Database::PreparedStatementBase *pStmt = Database::g_LoginDatabase.GetPrepareStatement(
                Database::LoginDatabaseSqlID::LOGIN_SEL_ACCOUNT_BY_EMAIL);
            pStmt->SerialValue(data->second, "123456@qq.com");

            auto callback = Database::g_LoginDatabase.AsyncQuery(pStmt).Then(
                // R"(select id, name, email, age, intro from account where email="123456@qq.com")",
                [&resp](Database::PreparedQueryResultSetPtr pResult) {
                    std::ostringstream ss;
                    ss << std::this_thread::get_id();
                    Log::Error("=======Query End:{}", ss.str());
                    if (pResult == nullptr)
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
                });

            _queryCallbackProcessor.AddCallback(std::move(callback));
            std::this_thread::sleep_for(std::chrono::seconds(3));
            std::ostringstream ss;
            ss << std::this_thread::get_id();
            Log::Warn("-------------------------------:{}", ss.str());
        });

    _router.AddHttpHandler(
        Http::HttpMethod::Get,
        "/user2",
        [](const Http::HttpRequest &request, Http::HttpResponse &resp) -> asio::awaitable<void> {
            auto data =
                Database::g_LoginDatabaseStmts.find(Database::LoginDatabaseSqlID::LOGIN_SEL_ACCOUNT_BY_EMAIL);
            if (data == Database::g_LoginDatabaseStmts.end())
            {
                resp.SetStatusCode(Http::StatusCode::InternalServerError);
                co_return;
            }
            Log::Debug("/user2 request");

            Database::PreparedStatementBase *pStmt = Database::g_LoginDatabase.GetPrepareStatement(
                Database::LoginDatabaseSqlID::LOGIN_SEL_ACCOUNT_BY_EMAIL);
            pStmt->SerialValue(data->second, "123456@qq.com");

            std::ostringstream ss;
            ss << std::this_thread::get_id();
            Log::Debug("**********request Query thread id:{}", ss.str());
            auto pResult = Database::g_LoginDatabase.SyncQuery(pStmt);
            // R"(select id, name, email, age, intro from account where email="123456@qq.com")");
            if (pResult == nullptr)
            {
                resp.SetStatusCode(Http::StatusCode::InternalServerError);
                co_return;
            }

            Database::Field *pFields = pResult->Fetch();
            uint32_t         id      = pFields[0];
            const char      *strName = pFields[1];
            const char      *email   = pFields[2];
            uint32_t         age     = pFields[3];
            std::string      intro   = pFields[4];

            resp.SetContent(
                std::format("id:{}, email:{}, name:{}, age:{}, intro:{}", id, email, strName, age, intro));
            Log::Debug("id:{}, email:{}, name:{}, age:{}, intro:{}", id, email, strName, age, intro);

            resp.SetStatusCode(Http::StatusCode::Ok);
        });

    _router.AddHttpHandler(
        Http::HttpMethod::Get,
        "/task",
        [this](const Http::HttpRequest &request, Http::HttpResponse &resp) -> asio::awaitable<void> {
            asio::post(_logicIoCtx, [&resp]() {
                Log::Error("logic post task");
                resp.SetContent(std::format("Hello post task"));
                resp.SetStatusCode(Http::StatusCode::Ok);
            });
            co_return;
        });
}

void HttpServer::OnScoketAccepted(Asio::socket &&socket, asio::io_context *pLogicIOCtx)
{
    auto pSession = std::make_shared<Http::HttpSession>(std::move(socket), pLogicIOCtx);
    pSession->SetRouter(_router);
    pSession->StartSession();

    AddNewSession(pSession);
}

void HttpServer::Update()
{
    Net::IServer::Update();

    _queryCallbackProcessor.ProcessReadyCallbacks();
}
