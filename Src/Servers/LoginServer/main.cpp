import Common;
import LoginServer;

int main()
{
    auto logDir = Util::GetExecutableDirectoryPath() / "log";
    if (!std::filesystem::exists(logDir))
    {
        std::filesystem::create_directories(logDir);
    }
    auto strLogFile = std::format("{}/HttpServer.html", logDir.string());

    Log::CLogger::GetLogger().InitLogger(strLogFile, 0, 10240, 10);

    try
    {
        LoginServer server("127.0.0.1", 10007);

        server.Start();
    }
    catch (const std::exception &exception)
    {
        Log::Critical("LoginServer服务器发生异常：{}", exception.what());
    }
    catch (...)
    {
        Log::Critical("Login服务器发生未知异常！！！");
    }

    std::stringstream ss;
    ss << std::this_thread::get_id();

    Log::Debug("main thread {}", ss.str());

    return 0;
}