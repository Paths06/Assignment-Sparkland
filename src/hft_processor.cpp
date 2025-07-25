#include "hft_processor.h"

HFTProcessor::HFTProcessor(const std::string& product_id, Logger& log) 
    : logger(log), csv_writer("ticker_data.csv", log), ws_client(product_id, log),
      ema_interval(5), price_ema_calc(0.2), mid_price_ema_calc(0.2) {
    
    last_ema_update = std::chrono::system_clock::now();
    
    // Set up WebSocket data callback
    ws_client.setDataCallback([this](const TickerData& ticker) {
        TickerData mutable_ticker = ticker;
        processTickerData(mutable_ticker);
    });
    
    logger.info("HFT Processor initialized for: " + product_id);
    logger.logTest("HFT_PROCESSOR_INIT", "PASSED", "Processor initialized for " + product_id);
}

HFTProcessor::~HFTProcessor() {
    stop();
}

void HFTProcessor::start() {
    if (running) {
        logger.warning("HFT Processor is already running");
        return;
    }
    
    running = true;
    ws_client.start();
    
    logger.info("HFT Processor started");
    logger.logTest("HFT_PROCESSOR_START", "PASSED", "Real-time processing started");
}

void HFTProcessor::stop() {
    if (!running) return;
    
    running = false;
    ws_client.stop();
    
    logStatistics();
    logger.info("HFT Processor stopped gracefully");
    logger.logTest("HFT_PROCESSOR_STOP", "PASSED", "Graceful shutdown completed");
}

void HFTProcessor::processTickerData(TickerData& ticker) {
    total_messages_processed++;
    
    ticker.sequence_number = total_messages_processed;
    
    ticker.price_ema = price_ema_calc.update(ticker.price);
    ticker.mid_price_ema = mid_price_ema_calc.update(ticker.mid_price);
    ema_updates_count++;
    
    csv_writer.writeTickerData(ticker);
    
    // Log every 25th processed message with EMA details
    if (total_messages_processed % 25 == 0) {
        logger.info(ticker.toLogString());
        logger.logTest("TICKER_PROCESSING", "PASSED", 
                      "Processed " + std::to_string(total_messages_processed) + 
                      " tickers with individual EMAs");
    }
    
    // Log periodic EMA progress every 100 messages
    if (total_messages_processed % 100 == 0) {
        logger.info("EMA Progress - Sequence #" + std::to_string(ticker.sequence_number) +
                   " | Total calculations: " + std::to_string(ema_updates_count) + 
                   " | Current Price EMA: $" + std::to_string(ticker.price_ema) +
                   " | Current Mid EMA: $" + std::to_string(ticker.mid_price_ema));
    }
}

// void HFTProcessor::updateEMAs(TickerData& ticker) {
//     ticker.price_ema = price_ema_calc.getCurrentEMA();
//     ticker.mid_price_ema = mid_price_ema_calc.getCurrentEMA();
// }

void HFTProcessor::logStatistics() const {
    logger.info("=== FINAL STATISTICS ===");
    logger.info("Total messages processed: " + std::to_string(total_messages_processed));
    logger.info("EMA calculations performed: " + std::to_string(ema_updates_count));
    logger.info("CSV records written: " + std::to_string(csv_writer.getRecordsWritten()));
    logger.info("WebSocket messages received: " + std::to_string(ws_client.getMessagesReceived()));
    logger.info("Final sequence number: " + std::to_string(total_messages_processed));
    
    // Calculate EMA efficiency
    double ema_efficiency = (total_messages_processed > 0) ? 
        (double(ema_updates_count) / double(total_messages_processed)) * 100.0 : 0.0;
    
    logger.info("EMA calculation efficiency: " + std::to_string(ema_efficiency) + "%");
    
    logger.logTest("FINAL_STATISTICS", "INFO", 
                  "Messages: " + std::to_string(total_messages_processed) + 
                  ", EMAs: " + std::to_string(ema_updates_count) + 
                  ", CSV records: " + std::to_string(csv_writer.getRecordsWritten()) +
                  ", Efficiency: " + std::to_string(ema_efficiency) + "%");
}