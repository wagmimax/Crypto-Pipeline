#include<iostream>
#include<WebSocketClient.h>

int main() {
    WebSocketClient BTC("stream.binance.us", "9443", "/ws/btcusdt@ticker");
    BTC.run();
}