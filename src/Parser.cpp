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
        
        try
        {
            json = nlohmann::json::parse(rawJSON);
            if(!json.contains("events") || json["events"].empty())
                continue;

            const auto& events = json["events"][0];
            
            if(!events.contains("trades") || events["trades"].empty())
                continue;

            const auto& trades = events["trades"];

            //coinbase sends batch trades in an array when they happen in the same microsecond
            for(const auto& trade : trades)
            {
                tradeData.push(TradeData{
                    trade["product_id"].get<std::string>(), 
                    trade["time"].get<std::string>(), 
                    std::stod(trade["price"].get<std::string>()), 
                    std::stod(trade["size"].get<std::string>())
                });
            }
        }
        catch(const std::exception& e)
        {
            std::cerr << "Error processing trade: " << e.what() << std::endl;
        }
    }
}