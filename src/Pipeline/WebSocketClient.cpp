#include<Pipeline/WebSocketClient.h>
#include<Pipeline/ConcurrentQueue.h>
#include<tools/profiler.h>
#include<format>

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

void WebSocketClient::connect(const std::vector<std::string>& pairs)
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

    std::string productIDs = "[";
    for(int i = 0; i < pairs.size(); i++) {
        productIDs += "\"";
        std::string ticker = pairs[i];
        productIDs += ticker.insert(3, 1, '-');
        productIDs += "\"";
        if(!(i == pairs.size() - 1)) {
            productIDs += ", ";
        }
    }
    productIDs += "]";

    std::cout << "Products: " << productIDs << "\n";

    std::string sub = std::format(R"({{
    "type": "subscribe",
    "product_ids": {},
    "channel": "market_trades"
    }})", productIDs);

    ws_.write(boost::asio::buffer(sub));
}



void WebSocketClient::run(const std::vector<std::string>& pairs)
{
    #ifdef TRACY_ENABLE
        tracy::SetThreadName("Websocket");
    #endif

    connect(pairs);
    std::cout << "Connected successfully! Waiting for data...\n";

    std::string stringJSON;

    while(true)
    {
        
    
        buffer_.consume(buffer_.size());      

        ws_.read(buffer_);     
        
        FRAME_MARK();
        PROFILE_SCOPE();

        stringJSON = boost::beast::buffers_to_string(buffer_.data());

        TimestampedMessage tm(stringJSON, std::chrono::high_resolution_clock::now());

        rawData.push(tm);
        //std::cout << "raw data pushed" << std::endl;
    }
}