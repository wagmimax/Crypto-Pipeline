#include<Pipeline/ConcurrentQueue.h>
#include<Pipeline/Parser.h>
#include<tools/profiler.h>
#include<simdjson.h>

void parseData()
{
    #ifdef TRACY_ENABLE
        tracy::SetThreadName("Parser");
    #endif
    std::cout << "Begin parsing" << std::endl;

    simdjson::dom::parser parser;
    while(true)
    {
        TimestampedMessage rawJSON = rawData.popValue();
        simdjson::padded_string stringJSON = rawJSON.json;

        FRAME_MARK();
        PROFILE_SCOPE();

        simdjson::dom::element json;
        parser.parse(stringJSON).get(json);

        auto trades = json["events"].get_array().at(0)["trades"];

        //coinbase sends batch trades in an array when they happen in the same microsecond
        for(const auto& trade : trades)
        {
            std::string productID;
            std::string time;
            std::string_view price;
            std::string_view size;

            if(trade["product_id"].get_string().get(productID)) continue;
            if(trade["time"].get_string().get(time)) continue;
            if(trade["price"].get_string().get(price)) continue;
            if(trade["size"].get_string().get(size)) continue;

            tradeData.push(TradeData{
                [] (std::string s) {s.erase(std::remove(s.begin(), s.end(), '-'), s.end()); return s;}
                (productID), 
                time, 
                std::stod(std::string(price)), 
                std::stod(std::string(size)),
                rawJSON.latencyTimestamp
            });
        }
    }
}