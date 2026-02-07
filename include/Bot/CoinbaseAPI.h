#pragma once
#include<string>

// Function to encode API secret key in base64
std::string getToken();

class CoinbaseAPI {
public:
    void getAccounts();
    void cancelOrders();
    void closePosition();
    void createOrder();
    void editOrder();

private:

};