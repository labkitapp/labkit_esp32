#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include "Config.h"
#include <ArduinoJson.h>
#include <AsyncWebSocket.h>

// Error codes for different types of failures
enum class ErrorCode {
    SUCCESS = 0,
    
    // General errors
    INVALID_PARAMETER,
    MISSING_PARAMETER,
    INVALID_JSON_FORMAT,
    UNKNOWN_COMMAND,
    
    // Pin/GPIO errors
    INVALID_PIN_NUMBER,
    INVALID_PIN_MODE,
    PIN_NOT_CONFIGURED_AS_OUTPUT,
    PIN_NOT_CONFIGURED_AS_INPUT,
    PIN_ALREADY_CONFIGURED,
    
    // I2C errors
    I2C_DEVICE_NOT_FOUND,
    I2C_READ_FAILED,
    I2C_WRITE_FAILED,
    I2C_NOT_INITIALIZED,
    I2C_INVALID_DEVICE_ADDRESS,
    I2C_INVALID_REGISTER,
    I2C_INVALID_BYTE_COUNT,
    
    // SPI errors
    SPI_INVALID_BYTE_COUNT,
    SPI_COMMAND_FAILED,
    SPI_NOT_INITIALIZED,
    SPI_NOT_AVAILABLE,
    
    // UART errors
    UART_INVALID_BAUDRATE,
    UART_MESSAGE_TOO_LONG,
    UART_NOT_AVAILABLE,
    
    // WiFi errors
    WIFI_CONNECTION_FAILED,
    WIFI_CREDENTIALS_MISSING,
    WIFI_ALREADY_CONNECTED,
    WIFI_NOT_CONNECTED,
    WIFI_SCAN_ERROR,
    
    // Memory errors
    MEMORY_ALLOCATION_FAILED,
    BUFFER_OVERFLOW,
    
    // System errors
    SYSTEM_INITIALIZATION_FAILED,
    HARDWARE_NOT_AVAILABLE,
    TIMEOUT_ERROR
};

// Error severity levels
enum class ErrorSeverity {
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

// Error context structure
struct ErrorContext {
    ErrorCode code;
    ErrorSeverity severity;
    String message;
    String details;
    String function;
    int line;
    JsonObject additionalData;
    
    ErrorContext(ErrorCode c, ErrorSeverity s, const String& msg, const String& func = "", int l = 0)
        : code(c), severity(s), message(msg), function(func), line(l) {}
};

class ErrorHandler {
private:
    static AsyncWebSocket* ws;
    static bool debugEnabled;
    
public:
    // Initialize the error handler
    static void initialize(AsyncWebSocket* websocket, bool debug = false);
    
    // Send error response over WebSocket
    static void sendError(ErrorCode code, const String& message, const String& function = "", 
                         const JsonObject& additionalData = JsonObject());
    
    // Send error response with context
    static void sendError(const ErrorContext& context);
    
    // Create error response JSON
    static String createErrorResponse(ErrorCode code, const String& message, 
                                    const String& function = "", const JsonObject& additionalData = JsonObject());
    
    // Log error to serial (if debug enabled)
    static void logError(const ErrorContext& context);
    
    // Check if an operation was successful
    static bool isSuccess(ErrorCode code);
    
    // Get human-readable error message
    static String getErrorMessage(ErrorCode code);
    
    // Get error severity string
    static String getSeverityString(ErrorSeverity severity);
    
    // Validate parameters and return appropriate error if invalid
    static ErrorCode validatePinNumber(int pin);
    static ErrorCode validatePinMode(int mode);
    static ErrorCode validateDeviceAddress(int address);
    static ErrorCode validateI2CByteCount(int bytes);
    static ErrorCode validateSPIByteCount(int bytes);
    static ErrorCode validateBaudrate(int baudrate);
    static ErrorCode validateSampleCount(int samples);
};

// Macro for easy error handling
#define HANDLE_ERROR(code, message, function) \
    ErrorHandler::sendError(code, message, function, __LINE__)

#define HANDLE_ERROR_WITH_DATA(code, message, function, data) \
    ErrorHandler::sendError(code, message, function, data)

// Macro for parameter validation
#define VALIDATE_PIN(pin) \
    do { \
        ErrorCode err = ErrorHandler::validatePinNumber(pin); \
        if (err != ErrorCode::SUCCESS) { \
            ErrorHandler::sendError(err, "Invalid pin number: " + String(pin), __FUNCTION__); \
            return; \
        } \
    } while(0)

#define VALIDATE_DEVICE_ADDRESS(addr) \
    do { \
        ErrorCode err = ErrorHandler::validateDeviceAddress(addr); \
        if (err != ErrorCode::SUCCESS) { \
            ErrorHandler::sendError(err, "Invalid device address: " + String(addr), __FUNCTION__); \
            return; \
        } \
    } while(0)

#define VALIDATE_I2C_BYTE_COUNT(bytes) \
    do { \
        ErrorCode err = ErrorHandler::validateI2CByteCount(bytes); \
        if (err != ErrorCode::SUCCESS) { \
            ErrorHandler::sendError(err, "Invalid I2C byte count: " + String(bytes), __FUNCTION__); \
            return; \
        } \
    } while(0)

#define VALIDATE_SPI_BYTE_COUNT(bytes) \
    do { \
        ErrorCode err = ErrorHandler::validateSPIByteCount(bytes); \
        if (err != ErrorCode::SUCCESS) { \
            ErrorHandler::sendError(err, "Invalid SPI byte count: " + String(bytes), __FUNCTION__); \
            return; \
        } \
    } while(0)

#endif // ERROR_HANDLER_H
