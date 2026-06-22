#ifndef COMMAND_HANDLERS_H
#define COMMAND_HANDLERS_H

#include "ErrorHandler.h"
#include "JsonResponseBuilder.h"
#include <ArduinoJson.h>
#include <AsyncWebSocket.h>

// Forward declarations
extern AsyncWebSocket ws;
extern int input_pins[][2];
extern int current_uart_baudrate;
extern uint8_t spirecv[5];

// Command handler function type
typedef void (*CommandHandler)(const JsonDocument &doc);

// I2C command handlers
void handleI2CTransactionCommand(const JsonDocument &doc);
void handleI2CSequenceCommand(const JsonDocument &doc);
void handleI2CRMWCommand(const JsonDocument &doc);
void handleI2CScanCommand(const JsonDocument &doc);
void handleI2CBusCheckCommand(const JsonDocument &doc);
void handleReadAllRegistersCommand(const JsonDocument &doc);

// SPI command handler
void handleSPITransactionCommand(const JsonDocument &doc);

// UART command handlers
void handleUARTBaudrateCommand(const JsonDocument &doc);
void handleUARTCommand(const JsonDocument &doc);

// GPIO command handlers
void handleGPIOOutCommand(const JsonDocument &doc);
void handleGPIOInCommand(const JsonDocument &doc);
void handleSetPinCommand(const JsonDocument &doc);

// System command handlers
void handleStatusCommand(const JsonDocument &doc);
void handleReadAnalogCommand(const JsonDocument &doc);
void handleWiFiCommand(const JsonDocument &doc);
void handleClearCommand(const JsonDocument &doc);
void handleArmCommand(const JsonDocument &doc);

// Pin monitoring function (called by main loop)
void checkAndNotifyPinChanges();

// Utility functions
bool connectToWiFi(const char *ssid, const char *password);
void clearWiFiCredentials();

#endif // COMMAND_HANDLERS_H
