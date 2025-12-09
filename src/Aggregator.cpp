#include<Aggregator.h>

void Aggregate()
{
    std::unordered_map<std::string, CandleData> candles;

    candles.emplace("BTC", CandleData("BTC", -1, -1, -1, -1, -1, "temp", "temp"));
    candles.emplace("ETH", CandleData("ETH", -1, -1, -1, -1, -1, "temp", "temp"));
    candles.emplace("SOL", CandleData("SOL", -1, -1, -1, -1, -1, "temp", "temp"));

    std::unordered_map<std::string, std::string> lastTimestamp;
    lastTimestamp["BTC"] = "";
    lastTimestamp["ETH"] = "";
    lastTimestamp["SOL"] = "";

    while(true)
    {
        TradeData currentTrade = tradeData.popValue();

        std::string ticker = currentTrade.ticker.substr(0, 3);
        std::string currentMinute = currentTrade.time.substr(14, 2);

        //coinbase sends old trade data sometimes. needs to be ignored
        if (!lastTimestamp[ticker].empty() && currentTrade.time < lastTimestamp[ticker])
            continue;
        lastTimestamp[ticker] = currentTrade.time;


        //initialize new candle when minute changes
        if(candles.at(ticker).startTime != currentMinute)
        {
            if(candles.at(ticker).startTime != "temp")
            {
                writeData(candles.at(ticker));
                std::cout << candles.at(ticker).startTime << " Candle Closed. OHLC: " << candles.at(ticker).open 
                    << " High: " << candles.at(ticker).high << " Low: " << candles.at(ticker).low 
                    << " Close: " << candles.at(ticker).close << " Timestamp: " << candles.at(ticker).timestamp << std::endl;
            }
            
            candles[ticker].open = currentTrade.price;
            candles[ticker].high = currentTrade.price;
            candles[ticker].low = currentTrade.price;
            candles[ticker].close = currentTrade.price;
            candles[ticker].volume = 0;
            candles[ticker].startTime = currentMinute;
            candles[ticker].timestamp = currentTrade.time;
        }

        //update current candle
        candles[ticker].close = currentTrade.price;
        candles[ticker].volume += currentTrade.volume;

        if(candles.at(ticker).high < currentTrade.price)
            candles[ticker].high = currentTrade.price;
        else if(candles.at(ticker).low > currentTrade.price)
            candles[ticker].low = currentTrade.price; 
    }
}