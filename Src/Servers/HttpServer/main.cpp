#include "HttpServer.h"
#include "Common/Util/Log.h"

#include "Common/Database/DatabaseImpl/LoginDatabase.h"

int main()
{
    Log::CLogger::GetLogger().InitLogger("log/HttpServer.html", 0, 10240, 10);

    Database::g_LoginDatabase.Open({"root", "cr11234", "test", "127.0.0.1", "3306"}, 1, 1);
    Database::g_LoginDatabase.PrepareStatements();

    try
    {
        auto pHttpServer = std::make_shared<HttpServer>("127.0.0.1", 10007);
        pHttpServer->Start();
    }
    catch (const std::exception &exception)
    {
        Log::Critical("Http服务器发生异常：{}", exception.what());
    }
    catch (...)
    {
        Log::Critical("Http服务器发生未知异常！！！");
    }

    std::stringstream ss;
    ss << std::this_thread::get_id();

    Log::Debug("main thread {}", ss.str());

    return 0;
}