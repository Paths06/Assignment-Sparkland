#include "test_runner.h"
#include "ema_calculator.h"
#include "ticker_data.h"
#include "json_parser.h"
#include "websocket_client.h"
#include <nlohmann/json.hpp>
#include <cassert>
#include <cmath>
#include <algorithm>

TestRunner::TestRunner(Logger& log) : logger(log), tests_passed(0), tests_failed(0) {}

void TestRunner::runAllTests() {
    logger.info("Starting comprehensive test");
    logger.logTest("TEST_SUITE", "STARTED", "Running all unit tests");
    
    testJSONParsing();
    testEMACalculation();
    testTickerDataStructure();
    testCSVFormatting();
    testWebSocketConnection();
    
    printTestSummary();
}

// Testing JSON Parsing
void TestRunner::testJSONParsing() {
    logger.info("Testing JSON parsing functionality");
    
    JSONParser parser(logger);
    
    // Test valid ticker JSON
    std::string valid_json = R"({
        "type": "ticker",
        "product_id": "BTC-USD",
        "price": "50000.00",
        "best_bid": "49999.00",
        "best_ask": "50001.00",
        "time": "2025-01-15T10:30:00.000000Z"
    })";
    
    try {
        TickerData ticker = parser.parseTickerMessage(valid_json);
        assertTrue(ticker.type == "ticker", "JSON_PARSE_TYPE");
        assertTrue(ticker.product_id == "BTC-USD", "JSON_PARSE_PRODUCT");
        assertEqual(50000.0, ticker.price, "JSON_PARSE_PRICE");
        assertEqual(50000.0, ticker.mid_price, "JSON_PARSE_MID_PRICE");
        
        logger.logTest("JSON_PARSING_VALID", "PASSED", "Successfully parsed valid ticker JSON");
    } catch (const std::exception& e) {
        logger.logTest("JSON_PARSING_VALID", "FAILED", e.what());
        tests_failed++;
    }
    
    // Test invalid JSON handling
    std::string invalid_json = "{ invalid json }";
    try {
        TickerData ticker = parser.parseTickerMessage(invalid_json);
        logger.logTest("JSON_PARSING_INVALID", "FAILED", "Should have thrown exception");
        tests_failed++;
    } catch (const std::exception&) {
        logger.logTest("JSON_PARSING_INVALID", "PASSED", "Correctly handled malformed JSON");
        tests_passed++;
    }
}

// Testing EMA calculation
void TestRunner::testEMACalculation() {
    logger.info("Testing EMA calculation accuracy");
    
    try {
        EMACalculator ema(0.2);
        
        // Test initialization
        assertTrue(!ema.isInitialized(), "EMA_INIT_STATE");
        
        // Test first value
        double first_ema = ema.update(100.0);
        assertTrue(ema.isInitialized(), "EMA_POST_INIT_STATE");
        assertEqual(100.0, first_ema, "EMA_FIRST_VALUE");
        
        // Test second value with formula
        double second_ema = ema.update(110.0);
        double expected = (110.0 * 0.2) + (100.0 * 0.8); // 22 + 80 = 102
        assertEqual(expected, second_ema, "EMA_SECOND_VALUE");
        
        // Test series calculation
        std::vector<double> test_prices = {100, 105, 102, 108, 95, 101, 99, 103};
        EMACalculator series_ema(0.3);
        
        double manual_ema = 100; // First value
        series_ema.update(100);
        
        for (size_t i = 1; i < test_prices.size(); ++i) {
            double calc_ema = series_ema.update(test_prices[i]);
            manual_ema = (test_prices[i] * 0.3) + (manual_ema * 0.7);
            
            assertTrue(std::abs(calc_ema - manual_ema) < 0.001, 
                      "EMA_SERIES_STEP_" + std::to_string(i));
        }
        
        logger.logTest("EMA_CALCULATION", "PASSED", 
                      "All EMA calculations verified against manual computation");
    } catch (const std::exception& e) {
        logger.logTest("EMA_CALCULATION", "FAILED", e.what());
        tests_failed++;
    }
}

void TestRunner::testTickerDataStructure() {
    logger.info("Testing TickerData structure");
    
    try {
        TickerData ticker;
        ticker.type = "ticker";
        ticker.product_id = "BTC-USD";
        ticker.price = 50000.50;
        ticker.best_bid = 49999.75;
        ticker.best_ask = 50001.25;
        ticker.price_ema = 49995.25;
        ticker.mid_price_ema = 49997.50;
        ticker.timestamp = std::chrono::system_clock::now();
        
        // Test mid price calculation
        ticker.calculateMidPrice();
        double expected_mid = (ticker.best_bid + ticker.best_ask) / 2.0;
        assertEqual(expected_mid, ticker.mid_price, "MID_PRICE_CALCULATION");
        
        // Test CSV generation
        std::string csv_row = ticker.toCSVRow();
        assertStringContains(csv_row, "ticker", "CSV_CONTAINS_TYPE");
        assertStringContains(csv_row, "BTC-USD", "CSV_CONTAINS_PRODUCT");
        
        // Test log string generation
        std::string log_string = ticker.toLogString();
        assertStringContains(log_string, "BTC-USD", "LOG_STRING_CONTAINS_PRODUCT");
        assertStringContains(log_string, "Price:", "LOG_STRING_CONTAINS_PRICE");
        
        logger.logTest("TICKER_DATA_STRUCTURE", "PASSED", "All structure methods working correctly");
    } catch (const std::exception& e) {
        logger.logTest("TICKER_DATA_STRUCTURE", "FAILED", e.what());
        tests_failed++;
    }
}

void TestRunner::testCSVFormatting() {
    logger.info("Testing CSV formatting");
    
    try {
        TickerData ticker;
        ticker.sequence_number = 42;
        ticker.type = "ticker";
        ticker.product_id = "BTC-USD";
        ticker.price = 50000.00;
        ticker.best_bid = 49999.50;
        ticker.best_ask = 50000.50;
        ticker.calculateMidPrice();
        ticker.price_ema = 49998.75;
        ticker.mid_price_ema = 49999.25;
        ticker.timestamp = std::chrono::system_clock::now();
        
        std::string csv = ticker.toCSVRow();
        
        // Count commas
        int comma_count = std::count(csv.begin(), csv.end(), ',');
        assertTrue(comma_count == 9, "CSV_COMMA_COUNT"); 
        
        // Check for required fields
        assertStringContains(csv, "42", "CSV_CONTAINS_SEQUENCE"); 
        assertStringContains(csv, "ticker", "CSV_CONTAINS_TYPE");
        assertStringContains(csv, "BTC-USD", "CSV_CONTAINS_PRODUCT");
        assertStringContains(csv, "50000.00", "CSV_CONTAINS_PRICE");
        
        logger.logTest("CSV_FORMATTING", "PASSED", "CSV format verified: " + csv);
    } catch (const std::exception& e) {
        logger.logTest("CSV_FORMATTING", "FAILED", e.what());
        tests_failed++;
    }
}

void TestRunner::testWebSocketConnection() {
    logger.info("Testing WebSocket connection parameters");
    
    // Test that we can create a WebSocket client without connecting
    try {
        WebSocketClient ws_client("BTC-USD", logger);
        assertTrue(true, "WEBSOCKET_CLIENT_CREATION");
        assertTrue(!ws_client.isConnected(), "WEBSOCKET_INITIAL_STATE");
        assertTrue(!ws_client.isRunning(), "WEBSOCKET_INITIAL_RUNNING_STATE");
        
        logger.logTest("WEBSOCKET_CLIENT_CREATION", "PASSED", "WebSocket client created successfully");
    } catch (const std::exception& e) {
        logger.logTest("WEBSOCKET_CLIENT_CREATION", "FAILED", e.what());
        tests_failed++;
    }
    
    // Test invalid smoothing factor handling
    try {
        EMACalculator invalid_ema(1.5); // Invalid: > 1.0
        logger.logTest("EMA_INVALID_ALPHA", "FAILED", "Should have thrown exception for alpha > 1.0");
        tests_failed++;
    } catch (const std::exception&) {
        logger.logTest("EMA_INVALID_ALPHA", "PASSED", "Correctly rejected invalid smoothing factor");
        tests_passed++;
    }
}

void TestRunner::assertTrue(bool condition, const std::string& test_name, const std::string& details) {
    if (condition) {
        logger.logTest(test_name, "PASSED", details);
        tests_passed++;
    } else {
        logger.logTest(test_name, "FAILED", details);
        tests_failed++;
    }
}

void TestRunner::assertEqual(double expected, double actual, const std::string& test_name, double tolerance) {
    bool equal = std::abs(expected - actual) < tolerance;
    std::string details = "Expected: " + std::to_string(expected) + ", Actual: " + std::to_string(actual);
    
    if (equal) {
        logger.logTest(test_name, "PASSED", details);
        tests_passed++;
    } else {
        logger.logTest(test_name, "FAILED", details);
        tests_failed++;
    }
}

void TestRunner::assertStringContains(const std::string& haystack, const std::string& needle, const std::string& test_name) {
    bool contains = haystack.find(needle) != std::string::npos;
    std::string details = "Looking for '" + needle + "' in '" + haystack + "'";
    
    if (contains) {
        logger.logTest(test_name, "PASSED", details);
        tests_passed++;
    } else {
        logger.logTest(test_name, "FAILED", details);
        tests_failed++;
    }
}

void TestRunner::printTestSummary() {
    int total_tests = tests_passed + tests_failed;
    std::string summary = "Tests Passed: " + std::to_string(tests_passed) + 
                         "/" + std::to_string(total_tests);
    
    logger.info("=== TEST SUMMARY ===");
    logger.info(summary);
    logger.logTest("TEST_SUITE", "COMPLETED", summary);
    
    if (tests_failed == 0) {
        logger.info("All tests passed successfully!");
        logger.logTest("TEST_RESULT", "SUCCESS", "All unit tests passed");
    } else {
        logger.error("Error " + std::to_string(tests_failed) + " tests failed!");
        logger.logTest("TEST_RESULT", "PARTIAL_FAILURE", std::to_string(tests_failed) + " tests failed");
    }
}