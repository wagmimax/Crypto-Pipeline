#pragma once
#include "Parser.h"
#include<deque>

enum struct TradeIntent
{
    NONE,
    LONG,
    SHORT
};

//Every strategy must return whether it wants to enter a trade or not,
//passing along an entry price level and stoploss percent. PaperBank 
//will handle all calculations based off the entry and stoploss percent
struct Trade
{   
    TradeIntent tradeIntent;
    double entryLevel;
    double stopLossPercent;
};

class Strategy
{
public:
    virtual Trade next(CandleData) {std::cout << "Entity" << std::endl; return Trade{};};

private:
    
};

class SupportResistance : public Strategy
{
public:
    Trade next(CandleData) override;
private:
    struct Level
    {
        double price;
        int touches;
        int age;
    };

    Level resistance;
    Level support;
    std::deque<CandleData> rollingWindow;
};