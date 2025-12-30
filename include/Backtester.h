#pragma once

#include "PaperAccount.h"
#include "Strategy.h"
#include <fstream>
#include <sstream>
#include <iomanip>

class Backtester
{
public:
    void run(Strategy& strategy);
    void run(Strategy* strategy)
    {
        if(!strategy) throw std::invalid_argument("null");
        run(*strategy);
    }

private:
    PaperAccount paperAccount;
};