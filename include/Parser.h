#pragma once
#include<iostream>
#include<string>
#include<vector>
#include<nlohmann/json.hpp>

struct CandleData
{
    std::string ticker;
    double open;
    double close; //keeps changing until the candle closes
    double high;
    double low;
    double volume;
    std::string startTime;

    CandleData(const std::string& ticker, double open, double close,
        double high, double low, double volume, const std::string& startTime):
        ticker(ticker), open(open), close(close), high(high), low(low), volume(volume), startTime(startTime) {}
};

struct TradeData
{
    std::string ticker;
    std::string time;
    double price;
    double volume;
    

    TradeData(const std::string& ticker, const std::string& time, double price, double volume):
        ticker(ticker), time(time), price(price), volume(volume) {}
};

void parseData();