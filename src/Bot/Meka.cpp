#pragma once
#include<Bot/Meka.h>
#include<Pipeline/ConcurrentQueue.h>

void Meka::start(Strategy& strategy, CoinbaseAPI& api, AccountManager& account) {

    while (true) {
        CandleData candle = candleData.popValue();
        Trade trade = strategy.next(candle);

        if(trade.tradeIntent != TradeIntent::NONE){
            account.placeTrade(api, trade);
  
        }
    }
}