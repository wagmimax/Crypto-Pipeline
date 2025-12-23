#include<Aggregator.h>

void Aggregate(Database& sqliteDB)
{
    std::unordered_map<std::string, CandleData> currentCandles;

    currentCandles.emplace("BTC", CandleData("BTC", -1, -1, -1, -1, -1, "temp", "temp", std::chrono::high_resolution_clock::now()));
    currentCandles.emplace("ETH", CandleData("ETH", -1, -1, -1, -1, -1, "temp", "temp", std::chrono::high_resolution_clock::now()));
    currentCandles.emplace("SOL", CandleData("SOL", -1, -1, -1, -1, -1, "temp", "temp", std::chrono::high_resolution_clock::now()));

    std::unordered_map<std::string, std::string> lastTimestamp;
    lastTimestamp["BTC"] = "";
    lastTimestamp["ETH"] = "";
    lastTimestamp["SOL"] = "";

    while(true)
    {
        TradeData currentTrade = tradeData.popValue();

        //auto latency = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - currentTrade.latencyTimestamp).count();
        //std::cout << "Socket -> Aggregator: " << latency << "\n";

        std::string ticker = currentTrade.ticker.substr(0, 3);
        std::string currentMinute = currentTrade.time.substr(14, 2);

        //coinbase sends old trade data sometimes. needs to be ignored
        if (!lastTimestamp[ticker].empty() && currentTrade.time < lastTimestamp[ticker])
            continue;
        lastTimestamp[ticker] = currentTrade.time;


        //initialize new candle when minute changes
        if(currentCandles.at(ticker).minute != currentMinute)
        {
            if(currentCandles.at(ticker).minute != "temp")
            {
                currentCandles[ticker].latencyTimestamp = currentTrade.latencyTimestamp;
                sqliteDB.writeData(currentCandles.at(ticker));
                std::cout << currentCandles.at(ticker).minute << " Candle Closed. OHLC: " << currentCandles.at(ticker).open 
                    << " High: " << currentCandles.at(ticker).high << " Low: " << currentCandles.at(ticker).low 
                    << " Close: " << currentCandles.at(ticker).close << " Timestamp: " << currentCandles.at(ticker).timestamp << std::endl;
            }
            
            currentCandles[ticker].open = currentTrade.price;
            currentCandles[ticker].high = currentTrade.price;
            currentCandles[ticker].low = currentTrade.price;
            currentCandles[ticker].close = currentTrade.price;
            currentCandles[ticker].volume = 0;
            currentCandles[ticker].minute = currentMinute;
            currentCandles[ticker].timestamp = currentTrade.time;
        }

        //update current candle
        currentCandles[ticker].close = currentTrade.price;
        currentCandles[ticker].volume += currentTrade.volume;

        if(currentCandles.at(ticker).high < currentTrade.price)
            currentCandles[ticker].high = currentTrade.price;
        else if(currentCandles.at(ticker).low > currentTrade.price)
            currentCandles[ticker].low = currentTrade.price; 

        candleData.push(currentCandles.at(ticker));
    }
}