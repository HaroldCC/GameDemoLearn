#include <asio.hpp>
#include <asio/awaitable.hpp>
#include <asio/use_awaitable.hpp>
#include <asio/detached.hpp>
#include <iostream>
#include <string>

using asio::awaitable;
using asio::co_spawn;
using asio::detached;
using asio::io_context;
using asio::use_awaitable;
using asio::ip::tcp;

awaitable<void> client_session(tcp::socket socket)
{
    try
    {
        std::string message = "Hello, server!";
        co_await asio::async_write(socket, asio::buffer(message), use_awaitable);

        char        data[1024];
        std::size_t n = co_await socket.async_read_some(asio::buffer(data), use_awaitable);

        std::cout << "Received: " << std::string(data, n) << "\n";
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception in client session: " << e.what() << "\n";
    }
}

int main()
{
    try
    {
        io_context io_ctx;

        tcp::resolver resolver(io_ctx);
        auto          endpoints = resolver.resolve("127.0.0.1", "12345");

        co_spawn(
            io_ctx,
            [&]() -> awaitable<void> {
                tcp::socket socket(io_ctx);
                co_await asio::async_connect(socket, endpoints, use_awaitable);

                co_await client_session(std::move(socket));
            },
            detached);

        io_ctx.run();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
