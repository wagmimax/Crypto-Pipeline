#pragma once
#include <Strategy.h>

Trade SupportResistance::next(const CandleData& candle)
{
    rollingWindow.push_back(candle);

    Trade trade{TradeIntent::NONE, -1, -1};    

    //check if breakouts are happening
    if(resistance.touches > touchThreshold && resistance.active && candle.close > resistance.price + resistance.price * tolerance) 
    {trade.tradeIntent = TradeIntent::LONG, trade.entryLevel = candle.close, trade.stopLossPercent = 0.65;}

    if(support.touches > touchThreshold && support.active && candle.close < support.price - support.price * tolerance) 
    {trade.tradeIntent = TradeIntent::SHORT, trade.entryLevel = candle.close, trade.stopLossPercent = 0.65;}
    //--------------------------------

    if(rollingWindow.size() >= 6)
    {   
        rollingWindow.pop_front();

        resistance.age++;
        support.age++;

        //check if middle candle is the swing high
        if(rollingWindow[0].high <= rollingWindow[2].high
            && rollingWindow[1].high <= rollingWindow[2].high 
            && rollingWindow[3].high <= rollingWindow[2].high
            && rollingWindow[4].high <= rollingWindow[2].high)
        {
            //if the candle high is in the bounds of resistance level and its tolerance, then label as a touch
            if(rollingWindow[2].high < resistance.price + resistance.price * tolerance &&
               rollingWindow[2].high > resistance.price - resistance.price * tolerance && resistance.active)
            {
                resistance.touches++;
                resistance.age = 0;
                if(DEBUGGING_ON) std::cout << "Resistance touched at " << rollingWindow[2].high << std::endl;
            }

            //new resistance being made
            else if(rollingWindow[2].high > resistance.price + resistance.price * tolerance)
            {
                resistance.price = rollingWindow[2].high;
                resistance.age = 0;
                resistance.touches = 0;
                if(DEBUGGING_ON) std::cout << "New resistance made at " << resistance.price << std::endl;
                resistance.active = true;
            }
        }

        //check if swing low
        if(rollingWindow[0].low >= rollingWindow[2].low
            && rollingWindow[1].low >= rollingWindow[2].low 
            && rollingWindow[3].low >= rollingWindow[2].low
            && rollingWindow[4].low >= rollingWindow[2].low)
        {
            //touch on support
            if(rollingWindow[2].low < support.price + support.price * tolerance &&
               rollingWindow[2].low > support.price - support.price * tolerance && support.active)
            {
                support.touches++;
                support.age = 0;
                if(DEBUGGING_ON) std::cout << "Support touched at " << rollingWindow[2].low << std::endl;;
            }
            //new support being made
            else if(rollingWindow[2].low < support.price - support.price * tolerance)
            {
                support.price = rollingWindow[2].low;
                support.age = 0;
                support.touches = 0;
                if(DEBUGGING_ON) std::cout << "New support made at " << support.price << std::endl;
                support.active = true;
            }
        }
        
        //if levels becomes old, replace them
        if(resistance.age > 100) {resistance.price = 0; resistance.age = 0; resistance.touches = 0; if(DEBUGGING_ON) std::cout << "Resistance expired\n"; resistance.active = false;}
        if(support.age > 100) {support.price = std::numeric_limits<double>::max(); support.age = 0; support.touches = 0; if(DEBUGGING_ON) std::cout << "Support expired\n"; support.active = false;}
    }      

    return trade;
}

void SupportResistance::reset()
{
    rollingWindow.clear();
    resistance.active = false;
    support.active = false;
}