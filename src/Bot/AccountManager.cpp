#include<Bot/AccountManager.h>


AccountManager::AccountManager(CoinbaseAPI& api): balance_(5), currentRisk_(2.0), RR_(3), wins(0), losses(0), DEBUGGING_ON(false), inPosition(false) {
    riskLevels[2.0] = balance_; 
    riskLevels[1.0] = balance_;
    riskLevels[0.5] = balance_;

    excessBal_ = api.listAccounts("USD") - balance_;

    auto fees = api.getTransactionSummary();
    makerFees_ = fees.makerFees;
    takerFees_ = fees.takerFees;

    std::cout << "maker fees: " << makerFees_ << " taker fees: " << takerFees_ << std::endl;
    std::cout << "Balance: " << balance_ << " Excess Balance: " << excessBal_ << std::endl;
}

void AccountManager::placeTrade(CoinbaseAPI& api, Trade& trade) {
    if(inPosition) return;

    double totalFees = makerFees_ + takerFees_;
    double targetProfitPercent = (trade.stopLossPercent + totalFees) * RR_ + totalFees;


    //this ensures our position size, with fees accounted for, will always lose us our currentRisk_% of our account upon stop loss hitting
    double positionSize = (balance_ * (currentRisk_/100)) / ((trade.stopLossPercent + makerFees_ + takerFees_)/100);

    double stopLossPrice;
    double targetProfitPrice;
    if(trade.tradeIntent == TradeIntent::LONG)
    {
        stopLossPrice = trade.entryLevel * (1 - (trade.stopLossPercent / 100));
        targetProfitPrice = trade.entryLevel * (1 + targetProfitPercent/100);
    }
    else if(trade.tradeIntent == TradeIntent::SHORT)
    {
        stopLossPrice = trade.entryLevel * (1 + (trade.stopLossPercent / 100));
        targetProfitPrice = trade.entryLevel * (1 - targetProfitPercent/100);
    }

    std::cout << "Position size: " << positionSize << std::endl;
    std::cout << "SL: " << stopLossPrice << std::endl;
    std::cout << "TP: " << targetProfitPrice << std::endl;


    inPosition = true;
    api.createOrder("ETH-USD", trade.tradeIntent, positionSize, trade.entryLevel, stopLossPrice, targetProfitPrice);
}

void AccountManager::adjustRisk()
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