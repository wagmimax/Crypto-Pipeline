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
    Strategy(): DEBUGGING_ON(false) {}

    virtual ~Strategy() = default;

    //every strategy subclass must update itself upon next candle
    virtual Trade next(const CandleData&) = 0;

    //every strategy must reset to its original state in between historical data files
    //files could be out of chronological order, or could be a completely different ticker
    //to prevent incorrect data, engine will call this function to reset the strategy and start from new
    virtual void reset() = 0;

    //if you want user controlled options for your strategy
    virtual void userInit() = 0;

    void enableDebugging()  { DEBUGGING_ON = true; };
    void disableDebugging() { DEBUGGING_ON = false; };
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
    touchThreshold(5)
    {}

    Trade next(const CandleData&) override;
    void reset() override;
    void userInit() override {};

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