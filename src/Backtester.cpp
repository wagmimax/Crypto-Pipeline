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
    loadHistoricalData(tempPath.getPath(), 5, "BNBUSDT");

    for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(tempPath.getPath()))
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

        if(auto* supportresistance = dynamic_cast<SupportResistance*>(&strategy))
            supportresistance->clearWindow();
    }


    //--------------------- Final Metrics output ---------------------------
    double wins = paperAccount.getWins();
    double losses = paperAccount.getLosses();
    double winrate = wins / (wins + losses) * 100;

    std::string finalBalStr = formatBalance(paperAccount.getBalance());
    std::string winrateStr = std::format("{:.2f}%", winrate);
    std::string winsStr = std::to_string(static_cast<int>(wins));
    std::string lossesStr = std::to_string(static_cast<int>(losses));

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

void Backtester::userControl()
{

}

//calls binance API for historical candle data and unzips of the files
void Backtester::loadHistoricalData(const std::filesystem::path& path, const int& granularity, const std::string_view& ticker)
{
    //mock command we are trying to replicate:
    //curl -s "https://data.binance.us/public_data/spot/monthly/klines/BNBUSDT/12h/BNBUSDT-12h-2023-01.zip" -o {tempdir}/BNBUS2DT-12h-2023-01-01.zip

    //downloads a full year worth of data
    for(int i{1}; i < 13; i++)
    {
        std::string month = (i < 10) 
            ? "0" + std::to_string(i) 
            : std::to_string(i); 

        std::string curlCommand = std::format(
            R"(curl -s  "https://data.binance.us/public_data/spot/monthly/klines/{}/{}m/{}-{}m-2025-{}.zip" -o )",
            ticker,
            granularity,
            ticker,
            granularity,
            month
        );

        std::string outputFile = std::format(
            R"({}/{}-{}m-2023-{}.zip)",
            path.string(),
            ticker,
            granularity,
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