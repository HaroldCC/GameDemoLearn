#include "DemoServer.h"
#include "Common/Util/Log.h"

int main()
{
    Log::CLogger::GetLogger().InitLogger("DemoServer.html", 0, 10240, 10);

    // DemoServer server("127.0.0.1", 12345);
    // server.Start();

    return 0;
}