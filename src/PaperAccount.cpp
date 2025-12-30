#include<PaperAccount.h>

void PaperAccount::enterPosition(Trade tradeSignal)
{
    if(!position.active)
    {
        position.entryPrice = tradeSignal.entryLevel;
        double totalFees = makerFees_ + takerFees_;

        //these will help with calculating PNL later 
        position.stopLossPercent = tradeSignal.stopLossPercent;
        position.targetProfitPercent = (tradeSignal.stopLossPercent + totalFees) * RR_ + totalFees;

        //this ensures our position size, with fees accounted for, will always lose us our currentRisk_% of our account upon stop loss hitting
        position.positionSize = (balance_ * (currentRisk_/100)) / ((tradeSignal.stopLossPercent + makerFees_ + takerFees_)/100);

        //calculate stoploss and targetprofit price levels (1:3 Risk:Reward by default)
        if(tradeSignal.tradeIntent == TradeIntent::LONG)
        {
            position.stopLossPrice = tradeSignal.entryLevel * (1 - (tradeSignal.stopLossPercent / 100));
            position.targetProfitPrice = tradeSignal.entryLevel * (1 + position.targetProfitPercent/100);
        }
        else if(tradeSignal.tradeIntent == TradeIntent::SHORT)
        {
            position.stopLossPrice = tradeSignal.entryLevel * (1 + (tradeSignal.stopLossPercent / 100));
            position.targetProfitPrice = tradeSignal.entryLevel * (1 - position.targetProfitPercent/100);
        }


        position.tradeType = tradeSignal.tradeIntent;
        position.active = true;

        std::cout << "POSITION SIZE: " << position.positionSize << "\n";
        std::cout << "IN TRADE, TP ENTRY SL: " << position.targetProfitPrice << " " << position.entryPrice << " " << position.stopLossPrice << std::endl;
        std::cout << "STOPLOSS%: " << position.stopLossPercent << " TARGETPROFIT%: " << position.targetProfitPercent << std::endl;

    }
}

void PaperAccount::checkOpenPositions(CandleData candle)
{
    if(position.active)
    {
        if(position.tradeType == TradeIntent::LONG)
        {
            //edgecase: if a candle is touching both tp and sl, omit the trade because we dont know which got hit first
            if(candle.high >= position.targetProfitPrice && candle.low <= position.stopLossPrice)
            {
                position.active = false;
                std::cout << "EDGECASE HIT, TRADE OMITTED\n";
                return;
            }

            //profit
            if(candle.high >= position.targetProfitPrice)
            {
                double profit = position.positionSize * position.targetProfitPercent/100;
                balance_ += profit;
                position.active = false;
                wins++;
                std::cout << "TAKEPROFIT HIT, BALANCE: " << balance_ << " PROFIT OF " << profit << std::endl;
            }

            //loss
            if(candle.low <= position.stopLossPrice)
            {
                double loss = position.positionSize * position.stopLossPercent/100;
                balance_ -= loss;
                position.active = false;
                losses++;
                std::cout << "STOPLOSS HIT, BALANCE: " << balance_ << " LOSS OF " <<  loss << std::endl;
            }
        }
        if(position.tradeType == TradeIntent::SHORT)
        {
            //edgecase: if a candle is touching both tp and sl, omit the trade because we dont know which got hit first
            if(candle.low <= position.targetProfitPrice && candle.high >= position.stopLossPrice)
            {
                position.active = false;
                std::cout << "EDGECASE HIT, TRADE OMITTED\n";
                return;
            }

            //profit
            if(candle.low <= position.targetProfitPrice)
            {
                double profit = position.positionSize * position.targetProfitPercent/100;
                balance_ += profit;
                position.active = false;
                wins++;
                std::cout << "TAKEPROFIT HIT, BALANCE: " << balance_ << " PROFIT OF " << profit << std::endl;
            }

            //loss
            if(candle.high >= position.stopLossPrice)
            {
                double loss = position.positionSize * position.stopLossPercent/100;
                balance_ -= loss;
                position.active = false;
                losses++;
                std::cout << "STOPLOSS HIT, BALANCE: " << balance_ << " LOSS OF " <<  loss << std::endl;
            }
        }

        adjustRisk();
    }
}

void PaperAccount::adjustRisk()
{
    if(currentRisk_ == 2.0)
    {
        if(balance_ > riskLevels[2.0])
            riskLevels[2.0] = balance_;
        else if(balance_ < riskLevels[2.0])
        {
            currentRisk_ = 1.0;
            riskLevels[1.0] = balance_;
        }
    }
    else if(currentRisk_ == 1.0)
    {
        if(balance_ >= riskLevels[2.0])
        {   
            currentRisk_ = 2.0;
            riskLevels[2.0] = balance_;
        }
        else if(balance_ < riskLevels[1.0])
        {
            currentRisk_ = 0.5;
            riskLevels[0.5] = balance_;
        }
    }
    else if(currentRisk_ == 0.5)
    {
        if(balance_ >= riskLevels[2.0])
        {
            currentRisk_ = 2.0;
            riskLevels[2.0] = balance_;
        }
        else if(balance_ >= riskLevels[1.0])
        {
            currentRisk_ = 1.0;
            riskLevels[1.0] = balance_;
        }
    }
}