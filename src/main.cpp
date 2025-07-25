#include "logger.h"
#include "test_runner.h"
#include "hft_processor.h"
#include <iostream>
#include <thread>
#include <csignal>
#include <chrono>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#endif

// Global variables for signal handling
static bool g_running = true;
static HFTProcessor* g_processor = nullptr;

void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ". Shutting down..." << std::endl;
    g_running = false;
    if (g_processor) {
        g_processor->stop();
    }
}

#ifdef _WIN32
class WSAInitializer {
public:
    WSAInitializer() {
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            std::cerr << "WSAStartup failed: " << result << std::endl;
            throw std::runtime_error("Failed to initialize Windows Sockets");
        }
        std::cout << "[INFO] Windows Sockets initialized successfully" << std::endl;
    }
    
    ~WSAInitializer() {
        WSACleanup();
        std::cout << "[INFO] Windows Sockets cleaned up" << std::endl;
    }
};
#endif

int main() {
#ifdef _WIN32
    try {
        WSAInitializer wsa_init;
#endif
        
        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);
        
        Logger logger("hft_app.log", "test_verification.log", LogLevel::INFO);
        
        logger.info("=== Coinbase HFT Ticker Application ===");
        
        try {
            // Run comprehensive test
            logger.info("Running pre-flight test...");
            TestRunner test_runner(logger);
            test_runner.runAllTests();
            
            // DEFINE PRODUCT HERE
            std::string target_product = "BTC-USD";  //CHANGE THIS LINE FOR DIFFERENT PRODUCTS
            // std::string target_product = "ETH-USD";  // Uncomment for Ethereum
            // std::string target_product = "SOL-USD";  // Uncomment for Solana
            
            // Initialize HFT processor
            logger.info("Initializing HFT processor for " + target_product + " trading pair");
            HFTProcessor processor(target_product, logger);
            g_processor = &processor;
            
            // Display startup information with dynamic product name
            logger.info("=== APPLICATION STARTUP ===");
            logger.info("Product: " + target_product);
            logger.info("EMA smoothing factor: 0.2 (20%)");
            logger.info("EMA calculation: With every message (Option B)");
            logger.info("Output files:");
            logger.info("  - ticker_data.csv (" + target_product + " market data)");
            logger.info("  - hft_app.log (application logs)");
            logger.info("  - test_verification.log (test results)");
            logger.info("Press Ctrl+C for graceful shutdown");
            logger.info("================================");
            
            // Start real-time processing
            logger.info("Starting real-time market data processing for " + target_product + "..."); 
            processor.start();
            
            // Keep main thread alive and log periodic statistics
            auto start_time = std::chrono::steady_clock::now();
            while (g_running) {
                std::this_thread::sleep_for(std::chrono::seconds(30));
                
                // Log periodic statistics every 30 seconds
                if (g_running) {
                    auto elapsed = std::chrono::duration_cast<std::chrono::minutes>(
                        std::chrono::steady_clock::now() - start_time).count();
                    
                    logger.info("Runtime: " + std::to_string(elapsed) + " minutes | " +
                               target_product + " messages processed: " + std::to_string(processor.getTotalMessagesProcessed()) + " | " +  // ✅ Dynamic!
                               "EMA updates: " + std::to_string(processor.getEMAUpdatesCount()));
                }
            }
            
            // Graceful shutdown
            logger.info("Initiating graceful shutdown for " + target_product + "...");
            processor.stop();
            
            // Final statistics for verification
            logger.info("=== FINAL STATISTICS FOR " + target_product + " ==="); 
            logger.info("Total messages processed: " + std::to_string(processor.getTotalMessagesProcessed()));
            logger.info("EMA updates performed: " + std::to_string(processor.getEMAUpdatesCount()));
            
            auto end_time = std::chrono::steady_clock::now();
            auto total_runtime = std::chrono::duration_cast<std::chrono::minutes>(end_time - start_time).count();
            logger.info("Total runtime: " + std::to_string(total_runtime) + " minutes");
            
            logger.logTest("APPLICATION_RUNTIME", "COMPLETED", 
                          "Runtime: " + std::to_string(total_runtime) + " minutes, " +
                          target_product + " messages: " + std::to_string(processor.getTotalMessagesProcessed()));
            
        } catch (const std::exception& e) {
            logger.error("Application error: " + std::string(e.what()));
            logger.logTest("APPLICATION_ERROR", "FAILED", e.what());
            return 1;
        }
        
        logger.info("Application shutdown completed successfully");
        logger.logTest("APPLICATION_SHUTDOWN", "SUCCESS", "Clean shutdown completed");
        
        std::cout << "\n=== Application Summary ===" << std::endl;
        std::cout << "Check 'test_verification.log' for test results" << std::endl;
        std::cout << "Check 'ticker_data.csv' for market data" << std::endl;
        std::cout << "Check 'hft_app.log' for detailed application logs" << std::endl;

#ifdef _WIN32
    } catch (const std::exception& e) {
        std::cerr << "WSA Initialization error: " << e.what() << std::endl;
        return 1;
    }
#endif
    
    return 0;
}