#include "ticker_data.h"
#include <sstream>
#include <iomanip>
#include <chrono>

TickerData::TickerData() 
    : price(0.0), best_bid(0.0), best_ask(0.0), mid_price(0.0), 
      price_ema(0.0), mid_price_ema(0.0), sequence_number(0) {}

std::string TickerData::toCSVRow() const {
    std::ostringstream oss;
    
    auto time_since_epoch = timestamp.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(time_since_epoch);
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(time_since_epoch) - 
                       std::chrono::duration_cast<std::chrono::microseconds>(seconds);
    
    auto time_t_val = std::chrono::system_clock::to_time_t(timestamp);
    
    // Format: YYYY-MM-DD HH:MM:SS.microseconds
    oss << std::put_time(std::gmtime(&time_t_val), "%Y-%m-%d %H:%M:%S");
    oss << "." << std::setfill('0') << std::setw(6) << microseconds.count();
    oss << "," << sequence_number
        << "," << type
        << "," << product_id
        << "," << std::fixed << std::setprecision(2) << price
        << "," << best_bid
        << "," << best_ask
        << "," << mid_price
        << "," << std::setprecision(6) << price_ema
        << "," << mid_price_ema;
    
    return oss.str();
}

std::string TickerData::toLogString() const {
    std::ostringstream oss;
    
    auto time_since_epoch = timestamp.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(time_since_epoch);
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(time_since_epoch) - 
                       std::chrono::duration_cast<std::chrono::microseconds>(seconds);
    
    auto time_t_val = std::chrono::system_clock::to_time_t(timestamp);
    
    oss << "#" << sequence_number << " " << product_id 
        << " [" << std::put_time(std::gmtime(&time_t_val), "%H:%M:%S");
    oss << "." << std::setfill('0') << std::setw(6) << microseconds.count() << "]";
    oss << " - Price: $" << std::fixed << std::setprecision(2) << price 
        << " | Mid: $" << mid_price
        << " | Price EMA: $" << std::setprecision(4) << price_ema
        << " | Mid EMA: $" << mid_price_ema;
    
    return oss.str();
}

void TickerData::calculateMidPrice() {
    mid_price = (best_bid + best_ask) / 2.0;
}