#include<iostream>
#include<Pipeline/DataPipeline.h>
#include<Backtester/Backtester.h>
#include<jwt/jwt.hpp>

typedef enum SelectedMode{NONE = 0, PIPELINE, BACKTESTER, BOT}SelectedMode;

int main() {

    std::cout << "Number of threads supported: " << std::thread::hardware_concurrency() << std::endl;

    int choice {};
    while(choice != PIPELINE && choice != BACKTESTER && choice != BOT)
    {
        std::cout <<"Select mode:\n"
                    "1 for pipeline\n"
                    "2 for backester\n"
                    "3 for bot\n";
        std::cin >> choice;
    }
    
    std::system("clear");

    const SelectedMode selectedMode = static_cast<SelectedMode>(choice);
 
    switch (selectedMode)
    {
        case PIPELINE:
        {
            Pipeline pipeline;
            pipeline.start();

        }break;
        case BACKTESTER:
        {
            Backtester backtester;
            SupportResistance supportResistance;
            backtester.run(supportResistance);
        }break;
        case BOT:
        {
            using namespace jwt::params;

            auto key = ""; //Secret to use for the algorithm
            //Create JWT object
            jwt::jwt_object obj{algorithm("HS256"), payload({{"some", "payload"}}), secret(key)};

            //Get the encoded string/assertion
            auto enc_str = obj.signature();
            std::cout << enc_str << std::endl;

            //Decode
            auto dec_obj = jwt::decode(enc_str, algorithms({"HS256"}), secret(key));
            std::cout << dec_obj.header() << std::endl;
            std::cout << dec_obj.payload() << std::endl;

            
        }break;
    }
}