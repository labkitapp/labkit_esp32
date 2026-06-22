#include "Config.h"
#include "WebSocketCommandHandler.h"
#include <vector>
#include <Arduino.h>

AsyncWebSocket* WebSocketCommandHandler::ws = nullptr;
std::map<String, CommandHandler> WebSocketCommandHandler::commandHandlers;
bool WebSocketCommandHandler::initialized = false;

void WebSocketCommandHandler::initialize(AsyncWebSocket* websocket) {
    ws = websocket;
    
    if (!initialized) {
        initializeDefaultCommands();
        initialized = true;
    }
}

void WebSocketCommandHandler::handleMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        data[len] = 0;
        DEBUG_PRINTF("DEBUG: Received WebSocket command: %s\n", (char*)data);
        
        // Try to parse as JSON first
#if LABKIT_I2C_TIMING_LOG
        const uint32_t i2cTimingWsT0 = micros();
#endif
        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, (char*)data);
#if LABKIT_I2C_TIMING_LOG
        const uint32_t i2cTimingDeserializeUs = micros() - i2cTimingWsT0;
#endif
        
        if (error) {
            ErrorHandler::sendError(ErrorCode::INVALID_JSON_FORMAT, 
                                  "Invalid JSON format: " + String(error.c_str()), __FUNCTION__);
            return;
        }
        
        if (!validateMessage(doc)) {
            ErrorHandler::sendError(ErrorCode::INVALID_JSON_FORMAT, 
                                  "Missing 'function' field in JSON message", __FUNCTION__);
            return;
        }
        
        String functionName = parseFunctionName(doc);
        DEBUG_PRINTF("DEBUG: Processing JSON command: %s\n", functionName.c_str());
#if LABKIT_I2C_TIMING_LOG
        if (functionName == "i2c_transaction") {
            I2C_TIMING_PRINTF("[I2C-TIMING] ws json_deserialize_us=%lu len=%u\n",
                              (unsigned long)i2cTimingDeserializeUs, (unsigned)len);
        }
#endif
        
        // Check if command is registered
        if (commandHandlers.find(functionName) != commandHandlers.end()) {
            // Execute the command handler
            commandHandlers[functionName](doc);
        } else {
            // Unknown command
            ErrorHandler::sendError(ErrorCode::UNKNOWN_COMMAND, 
                                  "Unknown command: " + functionName, __FUNCTION__);
        }
    }
}

void WebSocketCommandHandler::registerCommand(const String& command, CommandHandler handler) {
    commandHandlers[command] = handler;
    DEBUG_PRINTF("DEBUG: Registered command: %s\n", command.c_str());
}

bool WebSocketCommandHandler::isCommandRegistered(const String& command) {
    return commandHandlers.find(command) != commandHandlers.end();
}

std::vector<String> WebSocketCommandHandler::getRegisteredCommands() {
    std::vector<String> commands;
    for (const auto& pair : commandHandlers) {
        commands.push_back(pair.first);
    }
    return commands;
}

void WebSocketCommandHandler::initializeDefaultCommands() {
    // I2C commands
    registerCommand("i2c_transaction", handleI2CTransactionCommand);
    registerCommand("i2c_sequence", handleI2CSequenceCommand);
    registerCommand("i2c_rmw", handleI2CRMWCommand);
    registerCommand("i2c_scan", handleI2CScanCommand);
    registerCommand("i2c_bus_check", handleI2CBusCheckCommand);
    registerCommand("read_all_registers", handleReadAllRegistersCommand);

    // SPI commands
    registerCommand("spi_transaction", handleSPITransactionCommand);

    // GPIO commands
    registerCommand("gpio_out", handleGPIOOutCommand);
    registerCommand("gpio_in", handleGPIOInCommand);
    registerCommand("set_pin", handleSetPinCommand);

    // UART commands
    registerCommand("uart_baudrate", handleUARTBaudrateCommand);
    registerCommand("uart", handleUARTCommand);

    // WiFi commands
    registerCommand("wifi", handleWiFiCommand);
    registerCommand("clear", handleClearCommand);

    // System commands
    registerCommand("status", handleStatusCommand);
    registerCommand("read_analog", handleReadAnalogCommand);
    registerCommand("arm", handleArmCommand);

    DEBUG_PRINTF("DEBUG: Initialized %d command handlers\n", commandHandlers.size());
}

String WebSocketCommandHandler::parseFunctionName(const JsonDocument& doc) {
    if (doc.containsKey("function")) {
        return doc["function"].as<String>();
    }
    return "";
}

bool WebSocketCommandHandler::validateMessage(const JsonDocument& doc) {
    return doc.containsKey("function");
}
