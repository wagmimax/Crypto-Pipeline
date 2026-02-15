#pragma once
#define NOMINMAX    // windows min max macros break everything
#include<string>
#include"jwt/jwt.hpp"
#include"openssl/rand.h"
#include"curlcpp/curl_easy.h"   
#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"
#include "Backtester/Strategy.h"
#include "simdjson.h"
#include<fstream>
#include<iostream>
#include<format>
#include<list>
#include<array>

// When structs of data are needed from api response
namespace Responses {

    // Data of maker + taker fees for spot and 
    struct GetTransactionSummary {
        double makerFees;
        double takerFees;
    };
};

class CoinbaseAPI {
public:
    CoinbaseAPI(const std::string_view& key_name, const std::string_view& key_secret) : key_name(key_name), key_secret(key_secret) {}

    // Iterates through list of accounts. Returns available balance of currency passed in
    // Returns -1 if currency not found 
    double listAccounts(const std::string_view& currency) {
        std::string token = getToken("GET", "/api/v3/brokerage/accounts");
        
        std::ostringstream curlOutput;
        curl::curl_ios<std::ostringstream> ios(curlOutput);
        curl::curl_easy easy(ios);

        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());

        easy.add<CURLOPT_HTTPHEADER>(headers);
        easy.add<CURLOPT_URL>("https://api.coinbase.com/api/v3/brokerage/accounts");
        
        easy.perform();

        simdjson::dom::parser parser;
        simdjson::padded_string stringJSON = curlOutput.str();
        
        simdjson::dom::element json;
        parser.parse(stringJSON).get(json);

        auto accounts = json["accounts"].get_array();
        for(auto account : accounts) {
            std::string parsedVal;
            if(account["available_balance"]["currency"].get_string().get(parsedVal)) std::cout << "list accounts json error\n";    
            if(parsedVal == currency) {
                account["available_balance"]["value"].get_string().get(parsedVal);
                return std::stod(parsedVal);
            }
        }

        return -1;
    }

    // returns maker and taker fees for spot trading (in percent)
    Responses::GetTransactionSummary getTransactionSummary() {
        std::string token = getToken("GET", "/api/v3/brokerage/transaction_summary");
        
        std::ostringstream curlOutput;
        curl::curl_ios<std::ostringstream> ios(curlOutput);
        curl::curl_easy easy(ios);

        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());

        easy.add<CURLOPT_HTTPHEADER>(headers);
        easy.add<CURLOPT_URL>("https://api.coinbase.com/api/v3/brokerage/transaction_summary");
        
        easy.perform();

        simdjson::dom::parser parser;
        simdjson::padded_string stringJSON = curlOutput.str();
        
        simdjson::dom::element json;
        parser.parse(stringJSON).get(json);

        Responses::GetTransactionSummary response;
        std::string fees;

        json["fee_tier"]["taker_fee_rate"].get_string().get(fees);
        response.takerFees = (std::stod(fees)) * 100;

        json["fee_tier"]["maker_fee_rate"].get_string().get(fees);
        response.makerFees = (std::stod(fees)) * 100;

        return response;
    }
    
    // Create a limit order. Returns order_id string from response
    std::string createOrder(const std::string_view& ticker, TradeIntent tradeIntent, double quote, double entryPrice, double stopLoss, double targetProfit) {
        std::string token = getToken("POST", "/api/v3/brokerage/orders");

        // api response will be stored in curlOutput
        std::ostringstream curlOutput;
        curl::curl_ios<std::ostringstream> ios(curlOutput);
        curl::curl_easy easy(ios);

        boost::uuids::uuid uuid = boost::uuids::random_generator()();
    
        // Set orders to automatically expire in 60 seconds (might change this later)
        std::time_t time = std::time({}) + std::time_t{60};
        char timeString[std::size("yyyy-mm-ddThh:mm:ssZ")];
        std::strftime(std::data(timeString), std::size(timeString), "%FT%TZ", std::gmtime(&time));

        // limit orders require base size
        // Round to 6 decimal spots (too many decimals and coinbase rejects order, I think the max is 8?)
        auto setDecimalPrecision{ [](double num, int precision) -> double {
            int multiplier = 1;
            for(int i = 0; i < precision; i++) {
                multiplier *= 10;
            }

            long long normalizedLong = num * multiplier;
            return(static_cast<double>(normalizedLong) / multiplier);
        } };

        double baseSize = setDecimalPrecision(quote / entryPrice, 6);
        targetProfit = setDecimalPrecision(targetProfit, 2);
        stopLoss = setDecimalPrecision(stopLoss, 2);

        std::string jsonData = std::format(R"({{
            "client_order_id": "{}",
            "product_id": "{}",
            "side": "{}",
            "order_configuration": {{
                "limit_limit_gtd": {{
                    "base_size": "{}",
                    "limit_price": "{}",
                    "end_time": "{}"
                }}
            }},
            "attached_order_configuration": {{
                "trigger_bracket_gtc": {{
                    "limit_price": "{}",
                    "stop_trigger_price": "{}"
                }}
            }}
        }})", 
        boost::uuids::to_string(uuid), ticker, (tradeIntent == TradeIntent::LONG) ? "BUY" : "SELL" , baseSize, entryPrice, timeString, targetProfit, stopLoss);
        std::cout << jsonData << std::endl;

        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());
        headers = curl_slist_append(headers, ("Content-Type: application/json"));

        easy.add<CURLOPT_HTTPHEADER>(headers);
        easy.add<CURLOPT_URL>("https://api.coinbase.com/api/v3/brokerage/orders");
        easy.add<CURLOPT_POST>(1L);
        easy.add<CURLOPT_POSTFIELDS>(jsonData.c_str());

        easy.perform();

        simdjson::dom::parser parser;
        simdjson::padded_string stringJSON = curlOutput.str();
        
        simdjson::dom::element json;
        parser.parse(stringJSON).get(json);

        std::cout << curlOutput.str() << "\n\n\n";

        std::string order_id;
        if(json["success_response"]["order_id"].get_string().get(order_id)) std::cout << "create order json error\n";            
        return order_id;
    }

    // View an existing order's status
    std::string getOrder(const std::string_view& order_id) {
        std::string requestPath = "/api/v3/brokerage/orders/historical/" + std::string(order_id);
        std::string token = getToken("GET", requestPath);

        std::ostringstream curlOutput;
        curl::curl_ios<std::ostringstream> ios(curlOutput);
        curl::curl_easy easy(ios);

        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());

        easy.add<CURLOPT_HTTPHEADER>(headers);
        easy.add<CURLOPT_URL>(("https://api.coinbase.com" + requestPath).c_str());

        easy.perform();

        simdjson::dom::parser parser;
        simdjson::padded_string stringJSON = curlOutput.str();
        
        simdjson::dom::element json;
        parser.parse(stringJSON).get(json);

        std::string status;
        if(json["status"].get_string().get(status)) std::cout << "get order json error\n";
        return status;
    }

private:
    // JWT generator
    std::string getToken(const std::string_view& request_method, const std::string_view& request_path) {
        using namespace jwt::params;

        std::string uri;
        uri.reserve(request_method.size() + 1 + url.size() + request_path.size());
        uri.append(request_method);
        uri.push_back(' ');
        uri.append(url);
        uri.append(request_path);
        
        unsigned char nonce_raw[16];
        RAND_bytes(nonce_raw, sizeof(nonce_raw));
        std::stringstream nonce_stream;
        for(int i = 0; i < 16; i++) {
            nonce_stream << std::hex << std::setw(2) << std::setfill('0') << (int)nonce_raw[i];
        }
        std::string nonce = nonce_stream.str();

        jwt::jwt_object obj{algorithm("ES256"), secret(key_secret), headers({{"kid", key_name}, {"nonce", nonce}})};
        
        obj.add_claim("sub", key_name)
           .add_claim("nbf", std::chrono::system_clock::now())
           .add_claim("exp", std::chrono::system_clock::now() + std::chrono::seconds{120});

        obj.add_claim("iss", "cdp")
           .add_claim("uri", uri);

        std::string token = obj.signature();
        return token;
    }


    const std::string key_name;
    const std::string key_secret;
    const std::string url = "api.coinbase.com";
};