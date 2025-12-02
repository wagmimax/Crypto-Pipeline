#include<ConcurrentQueue.h>
#include<Parser.h>

void parseData()
{
    std::cout << "Begin parsing" << std::endl;
    std::string rawJSON;
    nlohmann::json json;
    using namespace std;

    while(true)
    {
        rawJSON = rawData.popCandle();
        json = nlohmann::json::parse(rawJSON);
        std::cout << json;
        

        //const auto& candles = json["candles"][0]; //ohlc data is inside "candles"
        //std::cout << candles << std::endl;
        // string ticker = candles["product_id"];
        // string open = candles["open"];
        // string close = candles["close"];
        // string high = candles["high"];
        // string low = candles["low"];
        // string volume = candles["volume"];
        // string startTime = candles["start"];

        // std::cout << ticker << "\n"; 

        // CandleData candle(ticker, stod(open), stod(close), stod(high), stod(low), stod(volume), startTime);
        // candleData.push(candle);
    }
}