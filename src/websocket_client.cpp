#include "websocket_client.h"

WebSocketClient::WebSocketClient(const std::string& product, Logger& log) 
    : logger(log), json_parser(log), product_id(product), 
      messages_received(0), parse_errors(0) {
    
    //Coinbase WebSocket URL
    std::string ws_url = "wss://ws-feed.exchange.coinbase.com";
    webSocket.setUrl(ws_url);
    
    setupCallbacks();
    
    logger.info("WebSocket client initialized for product: " + product_id);
    logger.info("Using WebSocket URL: " + ws_url);
}

WebSocketClient::~WebSocketClient() {
    stop();
}

void WebSocketClient::setDataCallback(std::function<void(const TickerData&)> callback) {
    data_callback = callback;
}

void WebSocketClient::start() {
    if (running) {
        logger.warning("WebSocket client is already running");
        return;
    }
    
    running = true;
    logger.info("Starting WebSocket connection to Coinbase Exchange");
    webSocket.start();
}

void WebSocketClient::stop() {
    if (!running) return;
    
    running = false;
    connected = false;
    webSocket.stop();
    
    logger.info("WebSocket client stopped");
    logger.info("Final statistics - Messages received: " + std::to_string(messages_received) + 
               ", Parse errors: " + std::to_string(parse_errors));
}

void WebSocketClient::setupCallbacks() {
    // Set the main callback handler
    webSocket.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
        switch (msg->type) {
            case ix::WebSocketMessageType::Message:
                logger.debug("Received message: " + msg->str.substr(0, 100) + "...");
                handleMessage(msg->str);
                break;
                
            case ix::WebSocketMessageType::Open:
                connected = true;
                logger.info("WebSocket connection opened successfully!");
                logger.logTest("WEBSOCKET_CONNECTION", "PASSED", "Connected to ws-feed.exchange.coinbase.com");
                
                // Wait a moment before subscribing
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                subscribeToTicker();
                break;
                
            case ix::WebSocketMessageType::Close:
                connected = false;
                logger.info("WebSocket connection closed - Code: " + std::to_string(msg->closeInfo.code) + 
                           ", Reason: " + msg->closeInfo.reason);
                logger.logTest("WEBSOCKET_DISCONNECT", "INFO", 
                              "Code: " + std::to_string(msg->closeInfo.code) + ", Reason: " + msg->closeInfo.reason);
                break;
                
            case ix::WebSocketMessageType::Error:
                logger.error("WebSocket error: " + msg->errorInfo.reason);
                logger.error("HTTP Status: " + std::to_string(msg->errorInfo.http_status));
                logger.logTest("WEBSOCKET_ERROR", "FAILED", 
                              "HTTP: " + std::to_string(msg->errorInfo.http_status) + 
                              " - " + msg->errorInfo.reason);
                break;
                
            case ix::WebSocketMessageType::Ping:
                logger.debug("WebSocket ping received");
                break;
                
            case ix::WebSocketMessageType::Pong:
                logger.debug("WebSocket pong received");
                break;
                
            case ix::WebSocketMessageType::Fragment:
                logger.debug("WebSocket fragment received");
                break;
        }
    });
}

void WebSocketClient::subscribeToTicker() {
    logger.info("Sending subscription request for " + product_id + "...");
    
    // Create subscription message
    nlohmann::json subscription;
    subscription["type"] = "subscribe";
    subscription["product_ids"] = nlohmann::json::array({product_id});
    subscription["channels"] = nlohmann::json::array({"ticker"});
    
    std::string sub_message = subscription.dump();
    logger.info("Subscription message: " + sub_message);
    
    // Send the subscription message
    ix::WebSocketSendInfo sendInfo = webSocket.send(sub_message);
    
    if (sendInfo.success) {
        logger.info("Subscription message sent successfully!");
        logger.info("Payload size: " + std::to_string(sendInfo.payloadSize) + " bytes");
        logger.logTest("TICKER_SUBSCRIPTION", "PASSED", "Subscribed to " + product_id);
    } else {
        logger.error("Failed to send subscription message");
        logger.logTest("TICKER_SUBSCRIPTION", "FAILED", "Failed to send subscription");
    }
    
    logger.info("Waiting for ticker data...");
}

void WebSocketClient::handleMessage(const std::string& message) {
    messages_received++;
    
    try {
        // Log the first few messages to see what we're getting
        if (messages_received <= 3) {
            logger.info("Message #" + std::to_string(messages_received) + ": " + message);
        }
        
        // Check if this is a subscription confirmation
        if (message.find("\"type\":\"subscriptions\"") != std::string::npos) {
            logger.info("Received subscription confirmation!");
            logger.logTest("SUBSCRIPTION_CONFIRMED", "PASSED", "Coinbase confirmed subscription");
            return;
        }
        
        TickerData ticker = json_parser.parseTickerMessage(message);
        
        if (ticker.type == "ticker" && data_callback) {
            logger.info("Processing ticker: " + ticker.product_id + 
                       " - Price: $" + std::to_string(ticker.price) +
                       " - Mid: $" + std::to_string(ticker.mid_price));
            data_callback(ticker);
        } else if (!ticker.type.empty() && ticker.type != "ticker") {
            logger.info("Received message type: " + ticker.type);
        }
        
        // Log progress every 25 messages
        if (messages_received % 25 == 0) {
            logger.info("Progress: " + std::to_string(messages_received) + " messages processed");
            logger.logTest("MESSAGE_PROCESSING", "PASSED", 
                          "Processed " + std::to_string(messages_received) + " messages");
        }
        
    } catch (const std::exception& e) {
        parse_errors++;
        logger.debug("Parse error: " + std::string(e.what()));
        
        // Show problematic message for first few errors
        if (parse_errors <= 3) {
            logger.debug("Problematic message: " + message.substr(0, 200) + "...");
        }
        
        if (parse_errors % 10 == 0) {
            logger.logTest("PARSE_ERRORS", "WARNING", 
                          "Total parse errors: " + std::to_string(parse_errors));
        }
    }
}