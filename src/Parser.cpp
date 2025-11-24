#include<ConcurrentQueue.h>
#include<Parser.h>

void parseData()
{
    std::cout << "Begin parsing" << std::endl;
    std::string stringJSON;
    nlohmann::json json;

    while(true)
    {
        stringJSON = rawData.pop();
        json = nlohmann::json::parse(stringJSON);
        const auto& k = json["k"]; // all data important data is in "k" object

        CandleData closedCandle(k["s"], k["o"], k["c"], k["h"], k["l"], k["x"], k["q"]);
        candleData.push(closedCandle);
    }
}