#include<iostream>
#include<Pipeline/DataPipeline.h>
#include<Backtester/Backtester.h>
#include<Bot/FIXapp.h>

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
            std::cout << "1";
            FIX::SessionSettings settings("../../data/config/quickfix.cfg");
            std::cout << "2";
            FIXapp app;
            std::cout << "3";
            FIX::FileStoreFactory storeFactory(settings);
            std::cout << "4";
            FIX::FileLogFactory logFactory(settings);
            std::cout << "5";

            FIX::SocketInitiator initiator(app, storeFactory, settings, logFactory);
            std::cout << "6";
            initiator.start();
            std::cout << "7";
            std::cin.get();
            std::cout << "8";
            initiator.stop();
        }break;
    }
}