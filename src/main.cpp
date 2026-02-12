#include<iostream>
#include<Pipeline/DataPipeline.h>
#include<Backtester/Backtester.h>
#include<Bot/TradingBot.h>
#include<Bot/CoinbaseAPI.h>
typedef enum SelectedMode{NONE = 0, PIPELINE, BACKTESTER, BOT}SelectedMode;

int main() {

    std::cout << "Number of threads supported: " << std::thread::hardware_concurrency() << std::endl;

    int choice {};
    while(choice != PIPELINE && choice != BACKTESTER && choice != BOT)
    {
        std::cout <<"Select mode:\n"
                    "1 for pipeline\n"
                    "2 for backester\n"
                    "3 for bot\n";
        std::cin >> choice;
    }
    
    std::system("clear");

    const SelectedMode selectedMode = static_cast<SelectedMode>(choice);
 
    switch (selectedMode)
    {
        case PIPELINE:
        {
            Pipeline pipeline;
            pipeline.start();

        }break;
        case BACKTESTER:
        {
            Backtester backtester;
            SupportResistance supportResistance;
            backtester.run(supportResistance);
        }break;
        case BOT:
        {
            CoinbaseAPI api;
            api.createOrder("ETH-USD", TradeIntent::LONG, 100, 1000, 900, 1100);
            
           
            //getToken("GET", "/api/v3/brokerage/accounts");
            //TradingBot bot;
            //bot.start();
            
            
        
            

        }break;
    }
}