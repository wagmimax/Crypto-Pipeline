#pragma once

#include "PaperAccount.h"
#include "Strategy.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>

class Backtester
{
public:
    void run(Strategy& strategy);
    void run(Strategy* strategy)
    {
        if(!strategy) throw std::invalid_argument("null strategy");
        run(*strategy);
    }

private:
    //user can edit backtester settings and choose which data to backtest
    void userControl();

    //use binance API to get historical candle data. stores in a temp file
    void loadHistoricalData(const int& granularity, const std::string_view& ticker, const std::vector<int>& years);

    std::filesystem::path path;
    PaperAccount paperAccount;
};