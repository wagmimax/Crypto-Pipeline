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
    std::string closed;
    std::string interval;

    CandleData(const std::string& ticker, double open, double close,
        double high, double low, double volume, const std::string& closed, const std::string& interval):
        ticker(ticker), open(open), close(close), high(high), low(low), volume(volume), closed(closed), interval(interval) {}
};

void parseData();