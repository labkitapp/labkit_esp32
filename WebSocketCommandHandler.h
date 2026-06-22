#ifndef WEBSOCKET_COMMAND_HANDLER_H
#define WEBSOCKET_COMMAND_HANDLER_H

#include <ArduinoJson.h>
#include <AsyncWebSocket.h>
#include <map>
#include <functional>
#include "ErrorHandler.h"
#include "CommandHandlers.h"

class WebSocketCommandHandler {
private:
    static AsyncWebSocket* ws;
    static std::map<String, CommandHandler> commandHandlers;
    static bool initialized;
    
public:
    // Initialize the command handler
    static void initialize(AsyncWebSocket* websocket);
    
    // Handle incoming WebSocket message
    static void handleMessage(void *arg, uint8_t *data, size_t len);
    
    // Register a command handler
    static void registerCommand(const String& command, CommandHandler handler);
    
    // Check if a command is registered
    static bool isCommandRegistered(const String& command);
    
    // Get list of registered commands
    static std::vector<String> getRegisteredCommands();
    
private:
    // Initialize default command handlers
    static void initializeDefaultCommands();
    
    // Parse JSON message and extract function name
    static String parseFunctionName(const JsonDocument& doc);
    
    // Validate JSON message format
    static bool validateMessage(const JsonDocument& doc);
};

#endif // WEBSOCKET_COMMAND_HANDLER_H
