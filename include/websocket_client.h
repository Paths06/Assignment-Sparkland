#pragma once
#include "ticker_data.h"
#include "logger.h"
#include "json_parser.h"
#include <ixwebsocket/IXWebSocket.h>
#include <queue>
#include <mutex>
#include <functional>
#include <atomic>

class WebSocketClient {
private:
    ix::WebSocket webSocket;
    std::mutex data_mutex;
    std::queue<TickerData> data_queue;
    
    Logger& logger;
    JSONParser json_parser;
    std::string product_id;
    std::atomic<bool> running{false};
    std::atomic<bool> connected{false};
    
    std::function<void(const TickerData&)> data_callback;
    
    // Statistics
    size_t messages_received;
    size_t parse_errors;

public:
    WebSocketClient(const std::string& product, Logger& log);
    ~WebSocketClient();
    
    void setDataCallback(std::function<void(const TickerData&)> callback);
    void start();
    void stop();
    bool isRunning() const { return running; }
    bool isConnected() const { return connected; }
    
    // Statistics
    size_t getMessagesReceived() const { return messages_received; }
    size_t getParseErrors() const { return parse_errors; }
    
private:
    void setupCallbacks();
    void subscribeToTicker();
    void handleMessage(const std::string& message);
};