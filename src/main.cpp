#include<iostream>
#include<WebSocketClient.h>
#include<ConcurrentQueue.h>
#include<thread>
#include<Database.h>
#include<Aggregator.h>

int main() {
    
    WebSocketClient coinbaseStream(
    "advanced-trade-ws.coinbase.com",
    "443",
    "/ws/v1");

    std::vector<std::string> pairs = {"BTC", "SOL", "ETH"};
    initTables(pairs);

    std::thread socketWorker(&WebSocketClient::run, &coinbaseStream);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    //flush any bad data in the beginning 
    rawData.clearData();
    tradeData.clearData();

    std::thread parseWorker(parseData);
    std::thread aggregateWorker(Aggregate);

    socketWorker.join();
    parseWorker.join();
    aggregateWorker.join();
}