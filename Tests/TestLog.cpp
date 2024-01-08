#include "Common/Util/Log.h"
#include "Common/Util/Util.h"

int main()
{
    Log::CLogger::GetLogger().InitLogger("log/test.html", 0, 10240, 10);
    Log::Debug("Hello Modules");
    Log::Info("真牛逼");

    int a = 1;
    Log::Debug("{}", a);

    Log::Warn("{}", Util::StringTo<int>("3").value_or(0));
    Log::Warn("{}", Util::StringTo<double>("3.13").value_or(0));
    Log::Warn("{}", Util::StringTo<bool>("3.13").value_or(0));
    Log::Warn("{}", Util::StringTo<float>("3.13").value_or(0));
}