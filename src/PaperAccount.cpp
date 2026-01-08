#pragma once
#include<PaperAccount.h>

std::string formatBalance(double value, int precision)
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    std::string s = oss.str();

    auto dotPos = s.find('.');
    int insertPosition = (dotPos == std::string::npos ? s.length() : dotPos) - 3;

    while (insertPosition > 0) {
        s.insert(insertPosition, ",");
        insertPosition -= 3;
    }

    s.insert(0, "$");

    return s;
}

void PaperAccount::enterPosition(Trade tradeSignal)
{
    //only 1 position permitted at a time
    if(position.active) return;
    
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

    if(DEBUGGING_ON) std::cout << "POSITION SIZE: " << position.positionSize << "\n";
    if(DEBUGGING_ON) std::cout << "IN TRADE, TP ENTRY SL: " << position.targetProfitPrice << " " << position.entryPrice << " " << position.stopLossPrice << std::endl;
    if(DEBUGGING_ON) std::cout << "STOPLOSS%: " << position.stopLossPercent << " TARGETPROFIT%: " << position.targetProfitPercent << std::endl;


}

void PaperAccount::checkOpenPositions(CandleData candle)
{
    if(!position.active) return;

    if(position.tradeType == TradeIntent::LONG)
    {
        //edgecase: if a candle is touching both tp and sl, omit the trade because we dont know which got hit first
        if(candle.high >= position.targetProfitPrice && candle.low <= position.stopLossPrice) [[unlikely]]
        {
            position.active = false;
            if(DEBUGGING_ON) std::cout << "EDGECASE HIT, TRADE OMITTED\n";
            return;
        }

        //profit
        if(candle.high >= position.targetProfitPrice)
        {

            double profit = position.positionSize * position.targetProfitPercent/100;
            balance_ += profit;
            position.active = false;
            wins++;
            if(DEBUGGING_ON) std::cout << "TAKEPROFIT HIT, BALANCE: " << balance_ << " PROFIT OF " << profit << std::endl;
            logTrade(position.tradeType, profit);
            adjustRisk();
        }

        //loss
        if(candle.low <= position.stopLossPrice)
        {
            double loss = position.positionSize * position.stopLossPercent/100;
            balance_ -= loss;
            position.active = false;
            losses++;
            if(DEBUGGING_ON) std::cout << "STOPLOSS HIT, BALANCE: " << balance_ << " LOSS OF " <<  loss << std::endl;
            logTrade(position.tradeType, -loss);
            adjustRisk();
        }
    }
    if(position.tradeType == TradeIntent::SHORT)
    {
        //edgecase: if a candle is touching both tp and sl, omit the trade because we dont know which got hit first
        if(candle.low <= position.targetProfitPrice && candle.high >= position.stopLossPrice) [[unlikely]]
        {
            position.active = false;
            if(DEBUGGING_ON) std::cout << "EDGECASE HIT, TRADE OMITTED\n";
            return;
        }

        //profit
        if(candle.low <= position.targetProfitPrice)
        {
            double profit = position.positionSize * position.targetProfitPercent/100;
            balance_ += profit;
            position.active = false;
            wins++;
            if(DEBUGGING_ON) std::cout << "TAKEPROFIT HIT, BALANCE: " << balance_ << " PROFIT OF " << profit << std::endl;
            logTrade(position.tradeType, profit);
            adjustRisk();
        }

        //loss
        if(candle.high >= position.stopLossPrice)
        {
            double loss = position.positionSize * position.stopLossPercent/100;
            balance_ -= loss;
            position.active = false;
            losses++;
            if(DEBUGGING_ON) std::cout << "STOPLOSS HIT, BALANCE: " << balance_ << " LOSS OF " <<  loss << std::endl;
            logTrade(position.tradeType, -loss);
            adjustRisk();
        }
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

void PaperAccount::logTrade(TradeIntent tradeType, double PNL)
{
    tabulate::Table log;

    log.add_row({
        "#" + std::to_string(wins+losses),                       // trade no. 
        (tradeType == TradeIntent::LONG) ? "LONG" : "SHORT",     // position type
        formatBalance(abs(PNL)),                                 // how much profitted or lost
        std::format("{:.1f}%", currentRisk_)                     // risk% of account on trade
    });

    log[0][2].format().font_color(
        (PNL > 0) ? tabulate::Color::green : tabulate::Color::red
    );

    std::cout << log << std::endl;
}