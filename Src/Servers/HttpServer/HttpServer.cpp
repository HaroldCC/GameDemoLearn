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
        });
    _router.AddHttpHandler(
        Http::HttpMethod::Get,
        "/user1",
        [](const Http::HttpRequest &request, Http::HttpResponse &resp) -> asio::awaitable<void> {
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

            Database::g_LoginDatabase.CoQuery(
                pStmt,
                // R"(select id, name, email, age, intro from account where email="123456@qq.com")",
                [&resp](Database::PreparedQueryResultSetPtr pResult) {
                    Log::Info("=======Query callback");
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

            std::ostringstream ss;
            ss << std::this_thread::get_id();
            Log::Debug("-------------------------------:{}", ss.str());
            std::this_thread::sleep_for(std::chrono::seconds(10));

            // Database::PreparedStatementBase *pStmt = Database::g_LoginDatabase.GetPrepareStatement(
            //     Database::LoginDatabaseSqlID::LOGIN_SEL_ACCOUNT_BY_EMAIL);
            // pStmt->SerialValue(data->second, "123456@qq.com");

            // //Database::PreparedQueryResultSetPtr pResult = Database::g_LoginDatabase.SyncQuery(pStmt);
            // _queryCallbackProcessor.AddCallback(Database::g_LoginDatabase.AsyncQuery(pStmt).Then(
            //     [&resp](Database::PreparedQueryResultSetPtr pResult) {
            //         if (nullptr == pResult)
            //         {
            //             resp.SetStatusCode(Http::StatusCode::InternalServerError);
            //             return;
            //         }

            //         Database::Field *pFields = pResult->Fetch();
            //         uint32_t         id      = pFields[0];
            //         const char      *strName = pFields[1];
            //         const char      *email   = pFields[2];
            //         uint32_t         age     = pFields[3];
            //         std::string      intro   = pFields[4];

            //         resp.SetContent(std::format("id:{}, email:{}, name:{}, age:{}, intro:{}",
            //                                     id,
            //                                     email,
            //                                     strName,
            //                                     age,
            //                                     intro));
            //         Log::Debug("id:{}, email:{}, name:{}, age:{}, intro:{}", id, email, strName, age, intro);

            //         resp.SetStatusCode(Http::StatusCode::Ok);
            //     }));

            // std::this_thread::sleep_for(std::chrono::seconds(5));
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
}

void HttpServer::OnScoketAccepted(Asio::socket &&socket)
{
    auto pSession = std::make_shared<Http::HttpSession>(std::move(socket), _router);
    pSession->StartSession();

    AddNewSession(pSession);
}

void HttpServer::Update()
{
    Net::IServer::Update();
}