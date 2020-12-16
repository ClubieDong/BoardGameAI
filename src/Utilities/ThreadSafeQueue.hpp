#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <deque>

template <typename T>
class ThreadSafeQueue
{
private:
    std::queue<T> _Queue;
    mutable std::mutex _Mtx;
    std::condition_variable _CV;

public:
    inline explicit ThreadSafeQueue() = default;
    ThreadSafeQueue(const ThreadSafeQueue &) = delete;
    ThreadSafeQueue &operator=(const ThreadSafeQueue &) = delete;
    ThreadSafeQueue(ThreadSafeQueue &&obj) = delete;
    ThreadSafeQueue &operator=(ThreadSafeQueue &&obj) = delete;

    inline void Push(const T &value)
    {
        std::lock_guard<std::mutex> lock(_Mtx);
        _Queue.push(value);
        _CV.notify_one();
    }

    inline void Push(T &&value)
    {
        std::lock_guard<std::mutex> lock(_Mtx);
        _Queue.push(std::move(value));
        _CV.notify_one();
    }

    inline T Pop()
    {
        std::unique_lock<std::mutex> lock(_Mtx);
        while (_Queue.empty())
            _CV.wait(lock);
        T front = std::move(_Queue.front());
        _Queue.pop();
        return front;
    }

    inline typename std::queue<T>::size_type GetSize() const
    {
        std::lock_guard<std::mutex> lock(_Mtx);
        return _Queue.size();
    }

    inline bool IsEmpty() const { return GetSize() == 0; }
};
