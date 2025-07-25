#pragma once
#include <string>
#include <fstream>
#include <mutex>
#include <sstream>
#include <iomanip>
#include <chrono>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
private:
    std::ofstream log_file;
    std::ofstream test_log_file;
    mutable std::mutex log_mutex;
    LogLevel min_level;
    
    std::string getCurrentTimestamp() const;
    std::string getCurrentTimestampMicroseconds() const;
    std::string levelToString(LogLevel level) const;

public:
    Logger(const std::string& log_filename = "hft_app.log", 
           const std::string& test_log_filename = "test_verification.log",
           LogLevel level = LogLevel::INFO);
    ~Logger();
    
    void log(LogLevel level, const std::string& message);
    void logTest(const std::string& test_name, const std::string& result, const std::string& details = "");
    
    // Convenience methods
    void debug(const std::string& message) { log(LogLevel::DEBUG, message); }
    void info(const std::string& message) { log(LogLevel::INFO, message); }
    void warning(const std::string& message) { log(LogLevel::WARNING, message); }
    void error(const std::string& message) { log(LogLevel::ERROR, message); }
};