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
        rawJSON = rawData.pop();
        json = nlohmann::json::parse(rawJSON);
        const auto& k = json["k"]; // all data important data is in "k" object

        string ticker = k["s"];
        string open = k["o"];
        string close = k["c"];
        string high = k["h"];
        string low = k["l"];
        string volume = k["q"];
        string closed = k["x"];
        string interval = k["i"];

        CandleData candle(ticker, stod(open), stod(close), stod(high), stod(low), stod(volume), closed, interval);
        candleData.push(candle);
    }
}