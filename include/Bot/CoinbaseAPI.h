#pragma once
#include<string>
#include"jwt/jwt.hpp"
#include"openssl/rand.h"
#include"curlcpp/curl_easy.h"   
#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"
#include "Backtester/Strategy.h"
#include<fstream>
#include<iostream>
#include<format>
#include<list>
#include<array>

namespace Responses {
    struct OrderResponse {

        std::string orderId;

    };
}

class CoinbaseAPI {
public:
    void listAccounts() {
        std::string token = getToken("GET", "/api/v3/brokerage/accounts");
        
        std::ostringstream curlOutput;
        curl::curl_ios<std::ostringstream> ios(curlOutput);
        curl::curl_easy easy(ios);

        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());

        easy.add<CURLOPT_HTTPHEADER>(headers);
        easy.add<CURLOPT_URL>("https://api.coinbase.com/api/v3/brokerage/accounts");
        
        try {
            easy.perform();

            std::cout << curlOutput.str() << std::endl;
        }
        catch (curl::curl_easy_exception &error) {
            std::cerr << error.what() << std::endl;
        }
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
        double base_size = quote / entryPrice;

        // TODO: Refactor this with simdjson
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
        boost::uuids::to_string(uuid), ticker, (tradeIntent == TradeIntent::LONG) ? "BUY" : "SELL" , base_size, entryPrice, timeString, targetProfit, stopLoss);

        std::cout << boost::uuids::to_string(uuid) << "\n";
        std::cout << jsonData << "\n";

        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());
        headers = curl_slist_append(headers, ("Content-Type: application/json"));

        easy.add<CURLOPT_HTTPHEADER>(headers);
        easy.add<CURLOPT_URL>("https://api.coinbase.com/api/v3/brokerage/orders");
        easy.add<CURLOPT_POST>(1L);
        easy.add<CURLOPT_POSTFIELDS>(jsonData.c_str());

        try {
            easy.perform();

            //TODO: Parse json and return order_id string
            std::cout << curlOutput.str() << std::endl;
           
        } catch (curl::curl_easy_exception &error) {
            std::cout << "ERROR" << std::endl;
            std::cerr << error.what() << std::endl;
        }
    }

private:
    std::string getToken(const std::string_view& request_method, const std::string_view& request_path) {
        using namespace jwt::params;
                
        std::ifstream inFile("../../secret.pem");
        std::stringstream keystream;
        keystream << inFile.rdbuf();

        std::cout << "Loading environment variables..." << std::endl;
        std::string key_name = std::getenv("COINBASE_KEY");
        std::string key_secret = keystream.str();
        std::cout << "Environment variables loaded successfully" << std::endl;

        std::cout << key_name << std::endl;
        std::cout << key_secret << std::endl;

        std::string url = "api.coinbase.com";
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

        try {
        std::string token = obj.signature();
        std::cout << token << std::endl;
        return token;
        } 
        catch (const std::exception& e) {
            std::cerr << "\nJWT error: " << e.what() << std::endl;
        }
    }


    std::string key_name;
    std::string key_secret;
};