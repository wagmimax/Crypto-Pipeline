#pragma once
#include"CoinbaseAPI.h"
#include"Backtester/Strategy.h"

class Meka {
public:

    void start(Strategy&, CoinbaseAPI&);
    
private:
    
};

/*
#pragma once
#include<Bot/Meka.h>
#include<ConcurrentQueue.h>

void Meka::start(Strategy& strategy, CoinbaseAPI& api) {

    while (true) {
        CandleData candle = candleData.popValue();
        Trade trade = strategy.next(candle);

        if(trade.tradeIntent == TradeIntent::NONE) continue;
    }
}
*/