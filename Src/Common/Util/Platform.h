/*************************************************************************
> File Name       : Platform.h
> Brief           : 平台预定义
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月08日  09时25分24秒
************************************************************************/
#pragma once
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(WIN64)
    #define OS_PLATFORM_WINDOWS
#else
    #define OS_PLATFORM_LINUX
#endif

#ifdef OS_PLATFORM_WINDOWS
    #include <io.h>
    #include <direct.h>
    #include <process.h>
    #include <ws2tcpip.h>
    #include <Windows.h>
    #include <mswsock.h>
    #include "Mstcpip.h"
    #include <ctime>
    #include <cstdarg>
    #include <TlHelp32.h>
    #pragma comment(lib, "ws2_32")
    #pragma comment(lib, "Mswsock")
#else
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <sys/epoll.h>
    #include <fcntl.h>
    #include <sys/resource.h>
    #include <string.h>
    #include <netinet/tcp.h>
    #include <signal.h>
    #include <sys/time.h>
    #include <sys/stat.h>
    #include <stdarg.h>
    #include <sys/ipc.h>
    #include <sys/shm.h>
    #include <dirent.h>
    #include <netdb.h>
#endif