#include<Database.h>

void initTables(std::vector<std::string>& pairs)
{
    std::string currentPair;
    sqlite3* db;
    sqlite3_open("../data/Database.db", &db); 

    if (sqlite3_open("../../../data/Database.db", &db) != SQLITE_OK) 
    {
        std::cout << "Failed to open database.\n";
        return;
    }

    for(int i = 0; i < pairs.size(); i++)
    {
        currentPair = pairs[i];
        std::string sql = "CREATE TABLE IF NOT EXISTS " + currentPair + " ("
        "timestamp TEXT PRIMARY KEY,"
        "open REAL, "
        "high REAL, "
        "low REAL, "
        "close REAL, "
        "volume REAL "
        ");";

        char* err = nullptr;
        int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err);

        if(rc != SQLITE_OK)
        {
            std::cout << "SQL error (" << rc << "): " << (err ? err : "unknown error") << "\n";
            if(err) sqlite3_free(err);
        }
        else
            std::cout << "Successfully created table" << std::endl;
    }

    sqlite3_close(db);
}

void writeData(CandleData candle)
{
    sqlite3* db;
    sqlite3_open("../../../data/Database.db", &db);

    std::string sql = "INSERT INTO " + candle.ticker + " (timestamp, open, high, low, close, volume) "
        "VALUES ('"
        + candle.timestamp + "', "
        + std::to_string(candle.open) + ", " 
        + std::to_string(candle.high) + ", " 
        + std::to_string(candle.low) + ", " 
        + std::to_string(candle.close) + ", " 
        + std::to_string(candle.volume) + ");";
    
    char* err = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err);

    if(rc != SQLITE_OK)
    {
        std::cout << "Error writing to database: " << err << std::endl;
        sqlite3_free(err);
    }
    else
        std::cout << "Wrote to database" << std::endl;

    sqlite3_close(db);
}