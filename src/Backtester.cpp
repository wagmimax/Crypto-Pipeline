#include <Backtester.h>

void Backtester::run(Strategy& strategy)
{
    std::ifstream csv("../../../data/BTCUSDT-5m-2025-01.csv");
    if(!csv.is_open())
    {
        std::cout << "Could not open file csv" << std::endl;
        return;
    }
    
    std::string row;

    //column headers
    std::getline(csv,row);
    std::cout << row << std::endl;

    while(std::getline(csv, row))
    {
        std::istringstream rowReader(row);
        CandleData candle;
        std::string field;


        // ------------- parse csv -> store as candle ----------------
        std::getline(rowReader, candle.timestamp, ','); //open timestamp

        std::getline(rowReader, field, ',');            //open price
        candle.open = std::stod(field);

        std::getline(rowReader, field, ',');            //high price
        candle.high = std::stod(field);

        std::getline(rowReader, field, ',');            //low price
        candle.low = std::stod(field);

        std::getline(rowReader, field, ',');            //close price
        candle.close = std::stod(field);

        std::getline(rowReader, field, ',');            //volume
        candle.volume = std::stod(field);
        // -----------------------------------------------------------

        std::cout << std::fixed << std::setprecision(2) 
        << "Candle OHLCV: " << candle.open << " " << candle.high << " " << candle.low << " " << candle.close << " " << candle.volume << "\n"; 

        //run candle through strategy and receive signal
        Trade trade = strategy.next(candle);

        //operate based off signal
        if(trade.tradeIntent != TradeIntent::NONE)
            paperAccount.enterPosition(trade);

        //check if candle triggers position closes
        paperAccount.checkOpenPositions(candle);
    }

    std::cout << "Final account balance: " << paperAccount.getBalance() << std::endl;
}