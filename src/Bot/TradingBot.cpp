#include<Bot/TradingBot.h>
#include<jwt/jwt.hpp>
#include<openssl/rand.h>
#include<iostream>
#include<fstream>

void TradingBot::start() {
    using namespace jwt::params;
            
    std::ifstream inFile("../../secret.pem");
    std::stringstream keystream;
    keystream << inFile.rdbuf();

    std::cout << "Loading environment variables..." << std::endl;
    std::string key_name = std::getenv("COINBASE_KEY");
    std::string key_secret = keystream.str();
    std::cout << "Environment variables loaded successfully" << std::endl;

    std::string request_method = "GET";
    std::string url = "api.coinbase.com";
    std::string request_path = "/api/v3/brokerage/accounts";
    std::string uri = request_method + " " + url + request_path;

    unsigned char nonce_raw[16];
    RAND_bytes(nonce_raw, sizeof(nonce_raw));
    std::stringstream nonce_stream;
    for(int i = 0; i < 16; i++) {
        nonce_stream << std::hex << std::setw(2) << std::setfill('0') << (int)nonce_raw[i];
    }
    std::string nonce = nonce_stream.str();

    jwt::jwt_object obj{algorithm("ES256"), secret(key_secret), headers({{"kid", key_name}, {"nonce", nonce}})};
    
    obj.add_claim("sub", key_name)
        .add_claim("iss", "cdp")
        .add_claim("nbf", std::chrono::system_clock::now())
        .add_claim("exp", std::chrono::system_clock::now() + std::chrono::seconds{120})
        .add_claim("uri", uri);

    try {
    std::string token = obj.signature();
    std::cout << token << std::endl;
    } 
    catch (const std::exception& e) {
        std::cerr << "\nJWT error: " << e.what() << std::endl;
    }
    
}