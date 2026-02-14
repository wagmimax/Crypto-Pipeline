#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <iostream>
#include <string>
#include "Pipeline/Parser.h"
#include "tools/moodycamel.h"

template <typename T>
class ConcurrentQueueLocked
{
public:
    void push(const T& value)
    {
        std::lock_guard<std::mutex> lock(m_);
        q_.push(value);
        cv_.notify_one();
    };

    T popValue()
    {
        std::unique_lock<std::mutex> lock(m_);
        cv_.wait(lock, [this]{ return !q_.empty(); });

        T value = std::move(q_.front());
        q_.pop();
        return value;
    };

    void clearData()
    {
        std::lock_guard<std::mutex> lock(m_);
        while(!q_.empty())
        {
            q_.pop();
        }
    }

private:
    std::mutex m_;
    std::condition_variable cv_;
    std::queue<T> q_;
};

template <typename T>
class ConcurrentQueue
{
public:
    void push(const T& value)
    {
        queue_.enqueue(value);
    }

    T popValue()
    {
        T value;

        while (!queue_.try_dequeue(value))
        {
            std::this_thread::yield();
        }
        return value;
    }

    void clearData()
    {
        T dummy;
        
        while (queue_.try_dequeue(dummy)){}
    }
private:
    moodycamel::ConcurrentQueue<T> queue_;
};

// Raw JSON from socket
extern ConcurrentQueue<TimestampedMessage> rawData;

// Parsed JSON now holding the trade
extern ConcurrentQueue<TradeData> tradeData;

// Closed 1m candles
extern ConcurrentQueue<CandleData> candleData;

// Used for logging completed candles
extern ConcurrentQueue<LoggerFormat> logs;
