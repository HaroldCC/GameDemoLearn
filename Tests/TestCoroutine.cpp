#include <asio.hpp>
#include <thread>
#include <deque>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <queue>
#include <mutex>
#include <condition_variable>

using asio::awaitable;
using asio::detached;
using asio::use_awaitable;
using asio::ip::tcp;

template <typename T>
class ThreadSafeQueue
{
public:
    ThreadSafeQueue() = default;

    void push(const T &value)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(value);
        }
        cond_.notify_one();
    }

    void push(T &&value)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(std::move(value));
        }
        cond_.notify_one();
    }

    bool try_pop(T &value)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty())
        {
            return false;
        }
        value = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    void wait_and_pop(T &value)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this] {
            return !queue_.empty();
        });
        value = std::move(queue_.front());
        queue_.pop();
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

private:
    mutable std::mutex      mutex_;
    std::queue<T>           queue_;
    std::condition_variable cond_;
};

class session : public std::enable_shared_from_this<session>
{
public:
    session(tcp::socket socket) : socket_(std::move(socket)), timer_(socket_.get_executor())
    {
    }

    void start()
    {
        co_spawn(
            socket_.get_executor(),
            [self = shared_from_this()] {
                return self->reader();
            },
            detached);

        co_spawn(
            socket_.get_executor(),
            [self = shared_from_this()] {
                return self->writer();
            },
            detached);
    }

    void send(const std::string &msg)
    {
        output_queue_.push(msg);
        timer_.cancel();
    }

    void read(std::string &msg)
    {
        std::unique_lock lock(input_queue_mutex_);
        if (!input_queue_.empty())
        {
            msg = std::move(input_queue_.front());
            input_queue_.pop();
        }
    }

private:
    awaitable<void> reader()
    {
        try
        {
            for (std::string read_msg;;)
            {
                std::cout << "reader wait" << std::endl;
                std::size_t n = co_await asio::async_read_until(socket_,
                                                                asio::dynamic_buffer(read_msg, 1024),
                                                                "\n",
                                                                use_awaitable);

                {
                    std::unique_lock lock(input_queue_mutex_);
                    input_queue_.push(read_msg.substr(0, n));
                    std::cout << "reader reacv msg：" << read_msg << std::endl;
                }
                read_msg.erase(0, n);
            }
        }
        catch (std::exception &)
        {
            stop();
        }
    }

    awaitable<void> writer()
    {
        try
        {
            while (socket_.is_open())
            {
                std::string msg;
                if (output_queue_.try_pop(msg))
                {
                    // There's a message in the queue, send it
                    std::cout << "writer send msg:" << msg << std::endl;
                    co_await asio::async_write(socket_, asio::buffer(msg), use_awaitable);
                }
                else
                {
                    // No message, wait for notification or send to abort
                    asio::error_code ec;
                    std::cout << "writer empty" << std::endl;
                    timer_.expires_after(std::chrono::hours(24));
                    co_await timer_.async_wait(asio::redirect_error(use_awaitable, ec));
                }
            }
        }
        catch (std::exception &)
        {
            stop();
        }
    }

    void stop()
    {
        socket_.close();
        timer_.cancel();
    }

    tcp::socket                  socket_;
    asio::steady_timer           timer_;
    std::queue<std::string>      input_queue_;
    std::mutex                   input_queue_mutex_;
    ThreadSafeQueue<std::string> output_queue_; // 使用线程安全队列
};

class server
{
public:
    server(asio::io_context &io_context, short port) : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
    {
        accept();
    }

    void accept()
    {
        acceptor_.async_accept([this](std::error_code ec, tcp::socket socket) {
            if (!ec)
            {
                auto new_session = std::make_shared<session>(std::move(socket));
                {
                    std::unique_lock lock(sessions_mutex_);
                    sessions_.insert(new_session);
                }
                new_session->start();
            }

            accept();
        });
    }

    void remove_session(std::shared_ptr<session> s)
    {
        std::unique_lock lock(sessions_mutex_);
        sessions_.erase(s);
    }

    void update_all_sessions()
    {
        // std::cout << "update_all_sessions:" << sessions_.size() << std::endl;
        std::unique_lock lock(sessions_mutex_);
        for (auto &s : sessions_)
        {
            std::string msg;
            s->read(msg);
            if (!msg.empty())
            {
                std::cout << "Received: " << msg << std::endl;
                s->send("Echo: " + msg + "\n");
            }
        }
    }

private:
    tcp::acceptor                      acceptor_;
    std::set<std::shared_ptr<session>> sessions_;
    std::mutex                         sessions_mutex_;
};

int main()
{
    try
    {
        asio::io_context io_context;

        server s(io_context, std::atoi("12345"));

        std::thread network_thread([&io_context] {
            io_context.run();
        });
        std::thread logic_thread([&s] {
            for (;;)
            {
                s.update_all_sessions();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });

        network_thread.join();
        logic_thread.join();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
