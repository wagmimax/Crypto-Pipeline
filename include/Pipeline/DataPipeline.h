#pragma once
#include"Pipeline/WebSocketClient.h"
#include"Pipeline/ConcurrentQueue.h"
#include<thread>
#include"Pipeline/Database.h"
#include"Pipeline/Aggregator.h"
#include"Pipeline/IPC.h"
#include"Pipeline/logger.h"

class Pipeline {
public:
    Pipeline(const std::vector<std::string>& pairs = {"ETHUSD"},
             const std::string& host = "advanced-trade-ws.coinbase.com",
             const std::string& port = "443",
             const std::string& path = "/ws/v1") :
    coinbaseStream_(host, port, path), pairs_(pairs), shouldStop_(false)
    { database_.initDB(pairs_); }

    ~Pipeline() { stop(); }

    //not copyable, not moveable
    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;
    Pipeline(const Pipeline&&) = delete;
    Pipeline& operator=(const Pipeline&&) = delete;

    template<typename... Threads>
    void start(Threads&&... threads) {
        (threads_.push_back(std::move(threads)), ...);

        threads_.emplace_back(&WebSocketClient::run, &coinbaseStream_, pairs_);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        //flush any bad data in the beginning 
        rawData.clearData();
        tradeData.clearData();

        threads_.emplace_back(parseData);
        threads_.emplace_back(Aggregate, std::ref(database_), std::ref(pairs_));
        //threads_.emplace_back(&NamedPipe::sendData, &server_);
        //threads_.emplace_back(logger);

        for(auto& thread : threads_) {
            if(thread.joinable()) {
                thread.join();
            }
        }
    }
    
    void stop() {};

private:
    WebSocketClient coinbaseStream_;
    Database database_;
    NamedPipe server_;
    std::vector<std::thread> threads_;
    std::vector<std::string> pairs_;
    std::atomic<bool> shouldStop_;
};