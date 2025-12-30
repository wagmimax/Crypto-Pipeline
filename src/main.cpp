#include<iostream>
#include<WebSocketClient.h>
#include<ConcurrentQueue.h>
#include<thread>
#include<Database.h>
#include<Aggregator.h>
#include<IPC.h>
#include<Backtester.h>

typedef enum SelectedMode{NONE = 0, PIPELINE, BACKTESTER}SelectedMode;

int main() {

    std::cout << "Number of threads supported: " << std::thread::hardware_concurrency() << std::endl;

    int choice{0};
    while(choice != PIPELINE && choice != BACKTESTER)
    {
        std::cout <<"Select mode:\n"
                    "1 for pipeline\n"
                    "2 for backester\n";
        std::cin >> choice;
    }
    
    const SelectedMode selectedMode = static_cast<SelectedMode>(choice);

    switch (selectedMode)
    {
        case PIPELINE:
        {
            WebSocketClient coinbaseStream(
            "advanced-trade-ws.coinbase.com",
            "443",
            "/ws/v1");

            NamedPipe server;
            Database sqliteDB;

            std::vector<std::string> pairs = {"BTC", "SOL", "ETH"};
            sqliteDB.initDB(pairs);

            std::thread socketWorker(&WebSocketClient::run, &coinbaseStream);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
            //flush any bad data in the beginning 
            rawData.clearData();
            tradeData.clearData();

            std::thread parseWorker(parseData);
            std::thread aggregateWorker(Aggregate, std::ref(sqliteDB));
            std::thread pipeWorker(&NamedPipe::sendData, &server);

            socketWorker.join();
            parseWorker.join();
            aggregateWorker.join();
            pipeWorker.join();
        }break;
        case BACKTESTER:
        {
            Backtester backtester;
            SupportResistance supportResistance;
            backtester.run(supportResistance);
        }break;
    }
}