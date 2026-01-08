#include <Backtester.h>
#include<thread>
#include<chrono>
#include<filesystem>

//finds your OS's temp files folder, and creates a directory there. 
//automatically cleans up (deletes) the directory once lifetime expires.
class TempPath
{
public:
    TempPath(const std::string& tempName = "TEMPCPPAPPFILES")
    {
        path_ = std::filesystem::temp_directory_path() / tempName;  
        std::filesystem::create_directories(path_);
    }

    ~TempPath()
    {
        std::filesystem::remove_all(path_);
    }

    std::filesystem::path getPath() const { return path_; }

    void printPath() const { std::cout << path_ << std::endl; }
    
private:
    std::filesystem::path path_;
};

void Backtester::run(Strategy& strategy)
{
    TempPath tempPath("BINANCEHISTORICALCANDLES");
    path = tempPath.getPath();

    strategy.userInit();
    userControl();

    for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(path))
    {
        if(dirEntry.path().string().substr(dirEntry.path().string().size() - 4) != ".csv")
            continue;

        std::this_thread::sleep_for(std::chrono::seconds(0));
        std::ifstream csv(dirEntry.path());

        if(!csv.is_open())
        {
            std::cout << "Could not open file csv" << std::endl;
            return;
        }
        //else {std::cout << "Opening file: " << dirEntry.path().string() << std::endl;}

        std::string row;

        //column headers
        std::getline(csv,row);

        while(std::getline(csv, row))
        {
            std::istringstream rowReader(row);
            CandleData candle;
            std::string field;


            // ------------- parse csv -> store as candle ----------------
            std::getline(rowReader, candle.timestamp, ','); //open timestamp

            std::getline(rowReader, field, ',');            //open price
            candle.open = std::stod(field);

            std::getline(rowReader, field, ',');            //high price
            candle.high = std::stod(field);

            std::getline(rowReader, field, ',');            //low price
            candle.low = std::stod(field);

            std::getline(rowReader, field, ',');            //close price
            candle.close = std::stod(field);

            std::getline(rowReader, field, ',');            //volume
            candle.volume = std::stod(field);
            // -----------------------------------------------------------


            //run candle through strategy and receive signal
            Trade trade = strategy.next(candle);

            //operate based off signal
            if(trade.tradeIntent != TradeIntent::NONE)
                paperAccount.enterPosition(trade);

            //check if candle triggers position closes
            paperAccount.checkOpenPositions(candle);
        }

        strategy.reset();
    }


    //--------------------- Final Metrics output ---------------------------
    double wins = paperAccount.getWins();
    double losses = paperAccount.getLosses();
    double winrate = wins / (wins + losses) * 100;

    std::string finalBalStr = formatBalance(paperAccount.getBalance());
    std::string winrateStr = std::format("{:.2f}%", winrate);
    std::string winsStr = std::to_string(static_cast<int>(wins));           //cast to int to
    std::string lossesStr = std::to_string(static_cast<int>(losses));       //remove decimal

    //organize metrics into a table for output
    tabulate::Table metricsTable;
    metricsTable.add_row({"Final Balance", "Wins", "Losses", "Winrate"});
    metricsTable.add_row({finalBalStr, winsStr, lossesStr, winrateStr});
    metricsTable[1][0].format().font_color
        ((paperAccount.getBalance() >= 50000) 
            ? tabulate::Color::green 
            : tabulate::Color::red);

    std::cout << metricsTable << std::endl;
    //--------------------------------------------------------------------------

}

//Currently has just about no error checking regarding user input
void Backtester::userControl()
{
    tabulate::Table message;
    std::string userInput;

    message.add_row({"Granularity", "Tickers Included", "Date Range"});
    message.add_row({"5m", "BTCUSDT, ETHUSDT, SOLUSDT", "2023 - 2025"});
    std::cout << message << "\n";

    while(userInput != "y" && userInput != "n")
    {
        std::cout << "Run premade dataset? (y/n)\n";
        std::cin >> userInput;
    }
    
    if(userInput == "y")
    {
        std::system("clear");
        loadHistoricalData(5, "BTCUSDT", {2023, 2024, 2025});
        loadHistoricalData(5, "ETHUSDT", {2023, 2024, 2025});
        loadHistoricalData(5, "SOLUSDT", {2023, 2024, 2025});
        std::system("clear");
    }
    else
    {
        std::system("clear");

        std::vector<int> years;
        std::string ticker;
        int granularity;
        userInput = "y";

        while(userInput != "n")
        {
            std::cout << "CUSTOM BACKTESTING DATASETS\n\n";

            std::cout << "Input ticker: ";
            std::cin >> ticker;

            std::cout << "Input granularity (in minutes): ";
            std::cin >> granularity;

            std::cout << "Input year(s) of data: ";
            std::string line;
            std::getline(std::cin >> std::ws, line);

            std::istringstream iss(line);
            for (int year; iss >> year; )
                years.push_back(year);

            std::system("clear");
            loadHistoricalData(granularity, ticker, years);
            std::system("clear");

            years.clear();

            std::cout << "Add more data? (y/n)";
            std::cin >> userInput;

            std::system("clear");
        }
    }

    std::cout << "Enter RR: ";
    std::cin >> userInput;

    if(userInput != "") paperAccount.setRR(std::stoi(userInput));
}

//calls binance API for historical candle data and unzips of the files
void Backtester::loadHistoricalData(const int& granularity, const std::string_view& ticker, const std::vector<int>& years)
{
    //mock command we are trying to replicate:
    //curl -s "https://data.binance.us/public_data/spot/monthly/klines/BNBUSDT/12h/BNBUSDT-12h-2023-01.zip" -o {tempdir}/BNBUS2DT-12h-2023-01-01.zip


    for(const auto& year : years)
    {
        //downloads a full year worth of data
        for(int day{1}; day < 13; day++)
        {
            std::string month = (day < 10) 
                ? "0" + std::to_string(day) 
                : std::to_string(day); 

            std::string curlCommand = std::format(
                R"(curl -s  "https://data.binance.us/public_data/spot/monthly/klines/{}/{}m/{}-{}m-{}-{}.zip" -o )",
                ticker,
                granularity,
                ticker,
                granularity,
                year,
                month
            );

            std::string outputFile = std::format(
                R"({}/{}-{}m-{}-{}.zip)",
                path.string(),
                ticker,
                granularity,
                year,
                month
            );

            std::string unzipCommand = std::format(
                R"(unzip -o "{}" -d "{}")",
                outputFile,
                path.string()
            );

            std::system((curlCommand + outputFile).c_str());
            std::system((unzipCommand).c_str());
        }
    }
}