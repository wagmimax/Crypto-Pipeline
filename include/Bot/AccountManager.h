#pragma once
#include"Backtester/Strategy.h"
#include"Bot/CoinbaseAPI.h"
#include<unordered_map>

class AccountManager {
public:
    AccountManager(CoinbaseAPI&);

    void placeTrade(CoinbaseAPI&, Trade&);

private:
    void adjustRisk();

    std::unordered_map<double, double> riskLevels;
    double currentRisk_;
    double balance_;
    double excessBal_;
    double makerFees_; 
    double takerFees_; 
    int RR_;
    int wins;
    int losses;
    bool DEBUGGING_ON;
    bool inPosition;
};