/*************************************************************************
> File Name       : Singleton.hpp
> Brief           : 单例
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年08月16日  14时16分05秒
************************************************************************/

#pragma once

template <typename T>
class Singleton
{
public:
    Singleton(const Singleton &)            = delete;
    Singleton &operator=(const Singleton &) = delete;
    Singleton(Singleton &&)                 = delete;
    Singleton &operator=(Singleton &&)      = delete;

    static Singleton &GetInstance()
    {
        static Singleton ins;
        return ins;
    }

private:
    Singleton()          = default;
    virtual ~Singleton() = default;
};