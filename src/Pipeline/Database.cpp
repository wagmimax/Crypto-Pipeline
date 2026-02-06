#include<Pipeline/Database.h>
#include<tools/profiler.h>

void Database::initDB(const std::vector<std::string>& pairs)
{

    if (sqlite3_open("../../data/Database.db", &db) != SQLITE_OK) 
    {
        std::cout << "Failed to open database.\n";
        return;
    }

    char* err_msg = nullptr;
    sqlite3_exec(db, "PRAGMA journal_mode = WAL", nullptr, nullptr, &err_msg);
    sqlite3_exec(db, "PRAGMA synchronous = NORMAL", nullptr, nullptr, &err_msg);

    if (err_msg) 
    {
        std::cerr << "SQLite error: " << err_msg << std::endl;
        sqlite3_free(err_msg);
    }

    for(int i = 0; i < pairs.size(); i++)
    {
        std::string currentPair = pairs[i];
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
}

void Database::writeData(CandleData candle)
{
    FRAME_MARK();
    PROFILE_SCOPE();
    
    auto t1 = std::chrono::high_resolution_clock::now();

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
    {
        //for recording standalone database write latency
        auto t2 = std::chrono::high_resolution_clock::now();
        LoggerFormat log(candle, t2);
        logs.push(log);

        // auto write_latency = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    
        // auto latency = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - candle.latencyTimestamp).count();
        // std::cout << "Socket -> Database Latency: " << latency << "μs\n";
    }
}