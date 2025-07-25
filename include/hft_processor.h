#pragma once
#include "ema_calculator.h"
#include "ticker_data.h"
#include "logger.h"
#include "csv_writer.h"
#include "websocket_client.h"
#include <chrono>
#include <atomic>
#include <thread>

class HFTProcessor {
private:
    EMACalculator price_ema_calc;
    EMACalculator mid_price_ema_calc;
    Logger& logger;
    CSVWriter csv_writer;
    WebSocketClient ws_client;
    
    std::chrono::system_clock::time_point last_ema_update;
    const std::chrono::seconds ema_interval;
    
    // Processing thread
    std::thread processing_thread;
    std::atomic<bool> running{false};
    
    // Statistics
    std::atomic<size_t> total_messages_processed{0};
    std::atomic<size_t> ema_updates_count{0};

public:
    HFTProcessor(const std::string& product_id, Logger& log);
    ~HFTProcessor();
    
    void start();
    void stop();
    void processTickerData(TickerData& ticker);
    
    // Statistics
    size_t getTotalMessagesProcessed() const { return total_messages_processed; }
    size_t getEMAUpdatesCount() const { return ema_updates_count; }
    
private:
    void processingLoop();
    void updateEMAs(TickerData& ticker);
    void logStatistics() const;
};