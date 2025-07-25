#include "json_parser.h"
#include <stdexcept>

JSONParser::JSONParser(Logger& log) : logger(log) {}

TickerData JSONParser::parseTickerMessage(const std::string& json_string) {
    try {
        nlohmann::json j = nlohmann::json::parse(json_string);
        
        if (!validateTickerJSON(j)) {
            throw std::invalid_argument("Invalid ticker JSON structure");
        }
        
        TickerData ticker;
        ticker.type = parseString(j, "type");
        ticker.product_id = parseString(j, "product_id");
        ticker.price = parsePrice(j, "price");
        ticker.best_bid = parsePrice(j, "best_bid");
        ticker.best_ask = parsePrice(j, "best_ask");
        ticker.time = parseString(j, "time");
        ticker.timestamp = std::chrono::system_clock::now();
        
        ticker.calculateMidPrice();
        
        return ticker;
        
    } catch (const std::exception& e) {
        logger.error("JSON parsing failed: " + std::string(e.what()));
        throw;
    }
}

bool JSONParser::validateTickerJSON(const nlohmann::json& j) const {
    return j.contains("type") && 
           j.contains("product_id") && 
           j.contains("price") && 
           j.contains("best_bid") && 
           j.contains("best_ask") &&
           j["type"] == "ticker";
}

double JSONParser::parsePrice(const nlohmann::json& j, const std::string& field) const {
    if (!j.contains(field)) {
        return 0.0;
    }
    
    if (j[field].is_string()) {
        return std::stod(j[field].get<std::string>());
    } else if (j[field].is_number()) {
        return j[field].get<double>();
    }
    
    return 0.0;
}

std::string JSONParser::parseString(const nlohmann::json& j, const std::string& field) const {
    if (!j.contains(field)) {
        return "";
    }
    
    return j[field].get<std::string>();
}