#include <iostream>
#include <format>

void TestHarold1(const char *str1, const char *str2)
{
    std::cout << std::format("{} {}", str1, str2);
}

double TestHarold(double a, double b)
{
    return a + b;
}

struct A
{
};

int main()
{
    A a;     // 默认构造
    A b = a; // 拷贝构造
    A c;     // 默认构造
    c = a;   // operator=

    return 0;
}