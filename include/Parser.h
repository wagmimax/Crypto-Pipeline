#pragma once
#include<iostream>
#include<string>
#include<vector>
#include<nlohmann/json.hpp>

struct CandleData
{
    std::string ticker;
    double openPrice;
    double closePrice; //keeps changing until the candle closes
    double high;
    double low;
    double volume;
    bool closed;

    CandleData(const std::string& ticker, double openPrice, double closePrice,
        double high, double low, double volume, bool closed):
        ticker(ticker), openPrice(openPrice), closePrice(closePrice), high(high), low(low), volume(volume), closed(closed) {}
};

void parseData();