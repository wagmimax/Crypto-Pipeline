#include <Strategy.h>

Trade SupportResistance::next(CandleData candle)
{
    rollingWindow.push_back(candle);

    if(rollingWindow.size() == 5)
    {   
        //check if middle candle is the swing high
        if(rollingWindow[0].high <= rollingWindow[2].high
            && rollingWindow[1].high <= rollingWindow[2].high 
            && rollingWindow[3].high <= rollingWindow[2].high
            && rollingWindow[4].high <= rollingWindow[2].high)
        {

            if(rollingWindow[2].high > resistance.price)
            {}
        }
    }    

    


    return Trade{TradeIntent::NONE, -1, -1};
}