#pragma once
#include "ticker_data.h"
#include "logger.h"
#include <nlohmann/json.hpp>

class JSONParser {
private:
    Logger& logger;

public:
    explicit JSONParser(Logger& log);
    
    TickerData parseTickerMessage(const std::string& json_string);
    bool validateTickerJSON(const nlohmann::json& j) const;
    
private:
    double parsePrice(const nlohmann::json& j, const std::string& field) const;
    std::string parseString(const nlohmann::json& j, const std::string& field) const;
};