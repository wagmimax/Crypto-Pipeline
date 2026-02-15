#include<iostream>
#include<Pipeline/DataPipeline.h>
#include<Backtester/Backtester.h>
#include<Bot/Meka.h>

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
            std::ifstream inFile("../../secret.pem");
            std::stringstream keystream;
            keystream << inFile.rdbuf();

            std::string key_name = std::getenv("COINBASE_KEY");
            std::string key_secret = keystream.str();


            CoinbaseAPI api(key_name, key_secret);
            AccountManager man(api); 
            CoinFlip coinFlip;
            Meka meka;
            
            Pipeline pipeline;
            pipeline.start(std::thread(&Meka::start, &meka, std::ref(coinFlip), std::ref(api), std::ref(man)));
        }break;
    }
}