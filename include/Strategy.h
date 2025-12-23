#pragma once
#include "Parser.h"

class Strategy
{
public:
    virtual void next(CandleData);
};

class SupportResistance : Strategy
{
public:
    void next(CandleData) override;
private:
    struct Level
    {
        double price;
        int touches;
        int age;
    };

    Level resistance;
    Level support;
    std::vector<CandleData> rollingWindow;
};