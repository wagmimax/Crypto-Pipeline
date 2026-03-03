#pragma once
#include<Bot/Meka.h>
#include<Pipeline/ConcurrentQueue.h>

void Meka::start(Strategy& strategy, CoinbaseAPI& api, AccountManager& account) {

    std::string order_id;
    
    // This loop is messy. will probably change internals later to make it more clean
    while (true) {
        if(!inTrade) {
            CandleData candle = candleData.popValue();
            Trade trade = strategy.next(candle);

            if(trade.tradeIntent != TradeIntent::NONE){
                std::cout << "TRADE SIGNAL DETECTED\n";
                order_id = account.placeTrade(api, trade);
                inTrade = (order_id != "") ? true : false; 
            }    
        }
        else {
            // every 1 minute, we check the status of our trade
            std::cout << "MONITORING OPEN TRADE...\n";
            std::this_thread::sleep_for(std::chrono::minutes(1));
            if(api.getOrder(order_id) == "FILLED") {
                std::cout << "TRADE EXIT FILLED\n";
                inTrade = false; 
                strategy.reset();
                candleData.clearData();
                std::cout << "NO LONGER IN TRADE, STRATEGY RESET, QUEUE CLEARED\n";
            } 
        }
    }
}