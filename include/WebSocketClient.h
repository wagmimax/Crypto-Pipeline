#pragma once
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <iostream>
#include <string>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;

class WebSocketClient 
{
public:
    WebSocketClient(const std::string& host, const std::string& port, const std::string& path);

    void connect();
    void run();
    void close();

private:
    std::string host_;
    std::string port_;
    std::string path_;

    boost::asio::io_context ioc_;
    boost::asio::ssl::context ssl_ctx_;
    boost::beast::websocket::stream<boost::beast::ssl_stream<boost::asio::ip::tcp::socket>> ws_;
    beast::flat_buffer buffer_;
};