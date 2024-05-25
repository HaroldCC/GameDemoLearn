/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*************************************************************************
> File Name       : ProducerConsumerQueue.hpp
> Brief           : 生产者消费者队列
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月28日  11时15分57秒
************************************************************************/
#pragma once
#include <condition_variable>
#include <mutex>
#include <queue>
#include <atomic>
#include <type_traits>

template <typename T>
class ProducerConsumerQueue
{
private:
    std::mutex              _queueLock;
    std::queue<T>           _queue;
    std::condition_variable _condition;
    std::atomic<bool>       _shutdown;

public:
    ProducerConsumerQueue<T>()
        : _shutdown(false)
    {
    }

    void Push(const T &value)
    {
        std::lock_guard<std::mutex> lock(_queueLock);
        _queue.push(std::move(value));

        _condition.notify_one();
    }

    bool Empty()
    {
        std::lock_guard<std::mutex> lock(_queueLock);

        return _queue.empty();
    }

    size_t Size() const
    {
        return _queue.size();
    }

    bool Pop(T &value)
    {
        std::lock_guard<std::mutex> lock(_queueLock);

        if (_queue.empty() || _shutdown)
        {
            return false;
        }

        value = _queue.front();

        _queue.pop();

        return true;
    }

    void WaitAndPop(T &value)
    {
        std::unique_lock<std::mutex> lock(_queueLock);

        _condition.wait(lock, [this]() -> bool
                        { return _queue.empty() && !_shutdown; });

        if (_queue.empty() || _shutdown)
        {
            return;
        }

        value = _queue.front();

        _queue.pop();
    }

    T WaitAndPop()
    {
        std::unique_lock<std::mutex> lock(_queueLock);
        T                            value{};

        _condition.wait(lock, [this]() -> bool
                        { return !_queue.empty() && !_shutdown; });
        // while (_queue.empty() && !_shutdown)
        //     _condition.wait(lock);

        if (_queue.empty() || _shutdown)
        {
            return value;
        }

        value = _queue.front();
        _queue.pop();
        return value;
    }

    void Clear()
    {
        std::unique_lock<std::mutex> lock(_queueLock);

        while (!_queue.empty())
        {
            T &value = _queue.front();

            DeleteQueuedObject(value);

            _queue.pop();
        }

        _shutdown = true;

        _condition.notify_all();
    }

private:
    template <typename E = T>
    typename std::enable_if<std::is_pointer<E>::value>::type DeleteQueuedObject(E &obj)
    {
        delete obj;
    }

    template <typename E = T>
    typename std::enable_if<!std::is_pointer<E>::value>::type DeleteQueuedObject(E const & /*packet*/)
    {
    }
};