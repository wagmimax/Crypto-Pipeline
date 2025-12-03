#include<WebSocketClient.h>
#include<ConcurrentQueue.h>

WebSocketClient::WebSocketClient(const std::string& host, const std::string& port, const std::string& path): 
host_(host),
port_(port),
path_(path),
ssl_ctx_(boost::asio::ssl::context::sslv23_client),
ws_(ioc_, ssl_ctx_)  
{
    ssl_ctx_.set_default_verify_paths();
    ssl_ctx_.set_verify_mode(boost::asio::ssl::verify_peer);
}

void WebSocketClient::connect()
{   
    boost::asio::ip::tcp::resolver resolver(ioc_);
    auto const results = resolver.resolve(host_, port_);

    boost::asio::connect(ws_.next_layer().next_layer(), results.begin(), results.end());

    // SNI
    if(! SSL_set_tlsext_host_name(ws_.next_layer().native_handle(), host_.c_str()))
        throw boost::system::system_error(
            boost::asio::error::operation_not_supported,
            "SNI Hostname Set Failed"
        );

    ws_.next_layer().handshake(boost::asio::ssl::stream_base::client);

    // send pong to keep connection
    ws_.control_callback(
        [&](boost::beast::websocket::frame_type frame_type,
            boost::beast::string_view payload)
        {
            if(frame_type == boost::beast::websocket::frame_type::ping)
                ws_.pong(boost::beast::websocket::ping_data{payload.data(), payload.size()});
        }
    );

    ws_.handshake(host_, path_);

    std::string sub = R"({
    "type": "subscribe",
    "product_ids": ["BTC-USD", "ETH-USD", "SOL-USD"],
    "channel": "market_trades"
    })";

    ws_.write(boost::asio::buffer(sub));
}



void WebSocketClient::run()
{
    connect();
        std::cout << "Connected successfully! Waiting for data...\n";

    while(true)
    {
        buffer_.consume(buffer_.size());      

        ws_.read(buffer_);                  
        std::string stringJSON = boost::beast::buffers_to_string(buffer_.data());

        rawData.push(stringJSON);
        //std::cout << "raw data pushed" << std::endl;
    }
}