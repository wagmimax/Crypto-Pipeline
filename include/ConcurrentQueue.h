#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <iostream>
#include <string>

template <typename T>
class ConcurrentQueue
{
public:
    void push(const T& value)
    {
        std::lock_guard<std::mutex> lock(m_);
        q_.push(value);
        cv_.notify_one();
    };

    T pop()
    {
        std::unique_lock<std::mutex> lock(m_);
        cv_.wait(lock, [this]{ return !q_.empty(); });

        T value = q_.front();
        q_.pop();
        return value;
    };

private:
    std::mutex m_;
    std::condition_variable cv_;
    std::queue<T> q_;
};

extern ConcurrentQueue<std::string> rawData;