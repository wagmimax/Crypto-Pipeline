#include<Aggregator.h>

void Aggregate()
{
    while(true)
    {
        TradeData currentTrade = tradeData.pop();

        std::cout << "Ticker: " << currentTrade.ticker << " Time: " << currentTrade.time 
        << " Price: " << currentTrade.price << " Volume: " << currentTrade.volume << std::endl;
    }
    
}