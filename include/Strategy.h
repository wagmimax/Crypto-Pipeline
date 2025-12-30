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
    Strategy(): DEBUGGING_ON(false){}
    virtual ~Strategy() = default;

    virtual Trade next(const CandleData&) = 0;

protected:
    bool DEBUGGING_ON;
};

class SupportResistance : public Strategy
{
public:
    SupportResistance():
    resistance({0,0,0,false}), 
    support({std::numeric_limits<double>::max(),0,0,false}), 
    tolerance(0.0005),
    touchThreshold(3)
    {}

    Trade next(const CandleData&) override;
    void clearWindow();

private:
    struct Level
    {
        double price;
        int touches;
        int age;
        bool active;
    };

    std::deque<CandleData> rollingWindow;
    Level resistance;
    Level support;
    double tolerance;
    int touchThreshold;
};