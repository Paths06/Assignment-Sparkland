#include "csv_writer.h"

CSVWriter::CSVWriter(const std::string& filename, Logger& log) 
    : logger(log), header_written(false), records_written(0) {
    
    csv_file.open(filename);
    if (!csv_file.is_open()) {
        logger.error("Failed to open CSV file: " + filename);
        throw std::runtime_error("Cannot open CSV file");
    }
    
    writeHeader();
    logger.info("CSV writer initialized " + filename);
}

CSVWriter::~CSVWriter() {
    if (csv_file.is_open()) {
        flush();
        csv_file.close();
        logger.info("CSV file closed. Total records written: " + std::to_string(records_written));
    }
}

void CSVWriter::writeHeader() {
    std::lock_guard<std::mutex> lock(csv_mutex);
    
    if (!header_written && csv_file.is_open()) {
        csv_file << "timestamp_microseconds,sequence_number,type,product_id,price,best_bid,best_ask,mid_price,price_ema,mid_price_ema\n";
        csv_file.flush();
        header_written = true;
        logger.debug("CSV header written");
    }
}

void CSVWriter::writeTickerData(const TickerData& ticker) {
    std::lock_guard<std::mutex> lock(csv_mutex);
    
    if (csv_file.is_open()) {
        csv_file << ticker.toCSVRow() << "\n";
        csv_file.flush();
        records_written++;
        
        // Log every 25th record for verification
        if (records_written % 25 == 0) {
            logger.info(" Record #" + std::to_string(records_written) + 
                       "written to sequence: " + std::to_string(ticker.sequence_number) + ")");
        }
    }
}

void CSVWriter::flush() {
    std::lock_guard<std::mutex> lock(csv_mutex);
    
    if (csv_file.is_open()) {
        csv_file.flush();
        logger.info("Manual flush completed\n");
    }
}
