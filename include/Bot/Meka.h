#pragma once
#include"CoinbaseAPI.h"
#include"Backtester/Strategy.h"
#include"Bot/AccountManager.h"  // this can cause circular include 

// Crypto Trading Bot
class Meka {
public:

    void start(Strategy&, CoinbaseAPI&, AccountManager&);
    
private:
    
};