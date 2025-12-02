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

void parseData();