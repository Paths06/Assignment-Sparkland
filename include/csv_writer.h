#pragma once
#include "ticker_data.h"
#include "logger.h"
#include <fstream>
#include <mutex>

class CSVWriter {
private:
    std::ofstream csv_file;
    std::mutex csv_mutex;
    Logger& logger;
    bool header_written;
    size_t records_written;

public:
    CSVWriter(const std::string& filename, Logger& log);
    ~CSVWriter();
    
    void writeHeader();
    void writeTickerData(const TickerData& ticker);
    void flush();
    size_t getRecordsWritten() const { return records_written; }
};
