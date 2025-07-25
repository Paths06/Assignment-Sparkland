#pragma once
#include <string>
#include <chrono>

struct TickerData {
    std::string type;
    std::string product_id;
    double price;
    double best_bid;
    double best_ask;
    double mid_price;
    std::string time;
    std::chrono::system_clock::time_point timestamp;
    double price_ema;
    double mid_price_ema;
    size_t sequence_number;
    
    TickerData();
    std::string toCSVRow() const;
    std::string toLogString() const;
    void calculateMidPrice();
};