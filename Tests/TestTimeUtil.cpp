#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include "Common/Util/TimeUtil.h"

using TimeUtil::Timer;
using namespace std::chrono_literals;

TEST_CASE("Timer Basic Functionality")
{
    Timer timer("", true);              // 假设这是您的 Timer 类的构造函数
    std::this_thread::sleep_for(100ms); // 模拟一些耗时操作
    double elapsed = timer.ElapsedMillisec();
    CHECK(elapsed >= 100.0); // 判断执行时间至少为 100 毫秒
    CHECK(elapsed < 200.0);  // 为了避免过度精确而导致的失败，设一个实际时间上限
}

TEST_CASE("Timer Reset Functionality")
{
    Timer timer("", true);
    std::this_thread::sleep_for(100ms);
    timer.Reset();                     // 重置 Timer
    std::this_thread::sleep_for(50ms); // 再次模拟耗时操作
    double elapsed = timer.ElapsedMillisec();
    CHECK(elapsed >= 50.0); // 检查重置后经过的时间
    CHECK(elapsed < 150.0); // 设立上限避免过度精确的问题
}
