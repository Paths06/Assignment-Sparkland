#pragma once
#include "logger.h"

class TestRunner {
private:
    Logger& logger;
    int tests_passed;
    int tests_failed;

public:
    explicit TestRunner(Logger& log);
    
    void runAllTests();
    void printTestSummary();
    
private:
    void testJSONParsing();
    void testEMACalculation();
    void testTickerDataStructure();
    void testCSVFormatting();
    void testWebSocketConnection();
    
    void assertTrue(bool condition, const std::string& test_name, const std::string& details = "");
    void assertEqual(double expected, double actual, const std::string& test_name, double tolerance = 0.001);
    void assertStringContains(const std::string& haystack, const std::string& needle, const std::string& test_name);
};