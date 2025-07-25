#include "logger.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>

Logger::Logger(const std::string& log_filename, const std::string& test_log_filename, LogLevel level)
    : min_level(level) {
    
    log_file.open(log_filename, std::ios::app);
    test_log_file.open(test_log_filename, std::ios::out);
    
    if (log_file.is_open()) {
        log(LogLevel::INFO, "=== HFT Application Started ===");
    }
    
    if (test_log_file.is_open()) {
        test_log_file << "=== TEST VERIFICATION LOG ===\n";
        test_log_file << "Generated at: " << getCurrentTimestampMicroseconds() << "\n";
        test_log_file << "Application: Coinbase HFT Ticker\n";
        test_log_file << "================================\n\n";
        test_log_file.flush();
    }
}

Logger::~Logger() {
    if (log_file.is_open()) {
        log(LogLevel::INFO, "=== HFT Application Ended ===");
        log_file.close();
    }
    
    if (test_log_file.is_open()) {
        test_log_file << "\n=== END OF TEST LOG ===\n";
        test_log_file.close();
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < min_level) return;
    
    std::lock_guard<std::mutex> lock(log_mutex);
    
    std::string timestamp = getCurrentTimestampMicroseconds();
    std::string level_str = levelToString(level);
    std::string log_line = "[" + timestamp + "] [" + level_str + "] " + message;
    
    // Log to file
    if (log_file.is_open()) {
        log_file << log_line << std::endl;
        log_file.flush();
    }
    
    // Log to console
    std::cout << log_line << std::endl;
}

void Logger::logTest(const std::string& test_name, const std::string& result, const std::string& details) {
    std::lock_guard<std::mutex> lock(log_mutex);
    
    if (test_log_file.is_open()) {
        test_log_file << "[" << getCurrentTimestampMicroseconds() << "] ";
        test_log_file << "TEST: " << test_name << " - " << result;
        if (!details.empty()) {
            test_log_file << " | Details: " << details;
        }
        test_log_file << std::endl;
        test_log_file.flush();
    }
}


std::string Logger::getCurrentTimestampMicroseconds() const {
    auto now = std::chrono::system_clock::now();
    
    auto time_since_epoch = now.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(time_since_epoch);
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(time_since_epoch) - 
                       std::chrono::duration_cast<std::chrono::microseconds>(seconds);
    
    auto time_t_val = std::chrono::system_clock::to_time_t(now);
    
    char buffer[64];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&time_t_val));

    std::ostringstream oss;
    oss << buffer << "." << std::setfill('0') << std::setw(6) << microseconds.count();
    
    return oss.str();
}

std::string Logger::getCurrentTimestamp() const {
    return getCurrentTimestampMicroseconds();
}

std::string Logger::levelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARN";
        case LogLevel::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}