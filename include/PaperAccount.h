#pragma once


class PaperAccount
{
public:
    PaperAccount(double balance, int leverage = 1, double makerFees = 0.4, double takerFees = 0.6):
    balance_(balance), leverage_(leverage), makerFees_(makerFees), takerFees_(takerFees)
    {}

    void setBalance(double amount) {balance_ = amount;}
    
    double getBalance() { return balance_; };

private:
    double calculatePosition(const double& stopLoss);

    double balance_;
    int leverage_;
    double makerFees_; 
    double takerFees_; 
    bool inLong_ = false;
    bool inShort_ = false;
};