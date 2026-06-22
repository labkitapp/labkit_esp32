#include "ErrorHandler.h"

AsyncWebSocket* ErrorHandler::ws = nullptr;
bool ErrorHandler::debugEnabled = false;

void ErrorHandler::initialize(AsyncWebSocket* websocket, bool debug) {
    ws = websocket;
    debugEnabled = debug;
}

void ErrorHandler::sendError(ErrorCode code, const String& message, const String& function, 
                            const JsonObject& additionalData) {
    if (!ws) return;
    
    String response = createErrorResponse(code, message, function, additionalData);
    ws->textAll(response);
    
    if (debugEnabled) {
        Serial.println("ERROR [" + getSeverityString(ErrorSeverity::ERROR) + "]: " + message);
        Serial.println("Function: " + function + ", Code: " + String((int)code));
    }
}

void ErrorHandler::sendError(const ErrorContext& context) {
    if (!ws) return;
    
    String response = createErrorResponse(context.code, context.message, context.function, context.additionalData);
    ws->textAll(response);
    
    logError(context);
}

String ErrorHandler::createErrorResponse(ErrorCode code, const String& message, 
                                       const String& function, const JsonObject& additionalData) {
    StaticJsonDocument<512> doc;
    doc["function"] = "error";
    doc["error_code"] = (int)code;
    doc["message"] = message;
    doc["severity"] = getSeverityString(ErrorSeverity::ERROR);
    
    if (function.length() > 0) {
        doc["source_function"] = function;
    }
    
    if (!additionalData.isNull()) {
        doc["details"] = additionalData;
    }
    
    String response;
    serializeJson(doc, response);
    return response;
}

void ErrorHandler::logError(const ErrorContext& context) {
    if (!debugEnabled) return;
    
    Serial.print("ERROR [");
    Serial.print(getSeverityString(context.severity));
    Serial.print("] ");
    Serial.print(context.message);
    Serial.print(" (Code: ");
    Serial.print((int)context.code);
    Serial.print(")");
    
    if (context.function.length() > 0) {
        Serial.print(" in ");
        Serial.print(context.function);
    }
    
    if (context.line > 0) {
        Serial.print(" at line ");
        Serial.print(context.line);
    }
    
    Serial.println();
    
    if (context.details.length() > 0) {
        Serial.println("Details: " + context.details);
    }
}

bool ErrorHandler::isSuccess(ErrorCode code) {
    return code == ErrorCode::SUCCESS;
}

String ErrorHandler::getErrorMessage(ErrorCode code) {
    switch (code) {
        case ErrorCode::SUCCESS:
            return "Success";
        case ErrorCode::INVALID_PARAMETER:
            return "Invalid parameter provided";
        case ErrorCode::MISSING_PARAMETER:
            return "Required parameter is missing";
        case ErrorCode::INVALID_JSON_FORMAT:
            return "Invalid JSON format";
        case ErrorCode::UNKNOWN_COMMAND:
            return "Unknown command";
        case ErrorCode::INVALID_PIN_NUMBER:
            return "Invalid pin number";
        case ErrorCode::INVALID_PIN_MODE:
            return "Invalid pin mode";
        case ErrorCode::PIN_NOT_CONFIGURED_AS_OUTPUT:
            return "Pin not configured as output";
        case ErrorCode::PIN_NOT_CONFIGURED_AS_INPUT:
            return "Pin not configured as input";
        case ErrorCode::PIN_ALREADY_CONFIGURED:
            return "Pin already configured";
        case ErrorCode::I2C_DEVICE_NOT_FOUND:
            return "I2C device not found";
        case ErrorCode::I2C_READ_FAILED:
            return "I2C read operation failed";
        case ErrorCode::I2C_WRITE_FAILED:
            return "I2C write operation failed";
        case ErrorCode::I2C_NOT_INITIALIZED:
            return "I2C not initialized";
        case ErrorCode::I2C_INVALID_DEVICE_ADDRESS:
            return "Invalid I2C device address";
        case ErrorCode::I2C_INVALID_REGISTER:
            return "Invalid I2C register address";
        case ErrorCode::I2C_INVALID_BYTE_COUNT:
            return "Invalid I2C byte count";
        case ErrorCode::SPI_INVALID_BYTE_COUNT:
            return "Invalid SPI byte count";
        case ErrorCode::SPI_COMMAND_FAILED:
            return "SPI command failed";
        case ErrorCode::SPI_NOT_INITIALIZED:
            return "SPI not initialized";
        case ErrorCode::SPI_NOT_AVAILABLE:
            return "SPI not available on this device";
        case ErrorCode::UART_INVALID_BAUDRATE:
            return "Invalid UART baudrate";
        case ErrorCode::UART_MESSAGE_TOO_LONG:
            return "UART message too long";
        case ErrorCode::UART_NOT_AVAILABLE:
            return "UART not available";
        case ErrorCode::WIFI_CONNECTION_FAILED:
            return "WiFi connection failed";
        case ErrorCode::WIFI_CREDENTIALS_MISSING:
            return "WiFi credentials missing";
        case ErrorCode::WIFI_ALREADY_CONNECTED:
            return "WiFi already connected";
        case ErrorCode::WIFI_NOT_CONNECTED:
            return "WiFi not connected";
        case ErrorCode::WIFI_SCAN_ERROR:
            return "WiFi scan failed";
        case ErrorCode::MEMORY_ALLOCATION_FAILED:
            return "Memory allocation failed";
        case ErrorCode::BUFFER_OVERFLOW:
            return "Buffer overflow";
        case ErrorCode::SYSTEM_INITIALIZATION_FAILED:
            return "System initialization failed";
        case ErrorCode::HARDWARE_NOT_AVAILABLE:
            return "Hardware not available";
        case ErrorCode::TIMEOUT_ERROR:
            return "Operation timed out";
        default:
            return "Unknown error";
    }
}

String ErrorHandler::getSeverityString(ErrorSeverity severity) {
    switch (severity) {
        case ErrorSeverity::INFO:
            return "INFO";
        case ErrorSeverity::WARNING:
            return "WARNING";
        case ErrorSeverity::ERROR:
            return "ERROR";
        case ErrorSeverity::CRITICAL:
            return "CRITICAL";
        default:
            return "UNKNOWN";
    }
}

// Parameter validation functions
ErrorCode ErrorHandler::validatePinNumber(int pin) {
    if (pin < 0 || pin >= MAX_PIN_ID) {
        return ErrorCode::INVALID_PIN_NUMBER;
    }
    return ErrorCode::SUCCESS;
}

ErrorCode ErrorHandler::validatePinMode(int mode) {
    if (mode < 0 || mode > 2) {
        return ErrorCode::INVALID_PIN_MODE;
    }
    return ErrorCode::SUCCESS;
}

ErrorCode ErrorHandler::validateDeviceAddress(int address) {
    if (address < 1 || address > 127) {
        return ErrorCode::I2C_INVALID_DEVICE_ADDRESS;
    }
    return ErrorCode::SUCCESS;
}

ErrorCode ErrorHandler::validateI2CByteCount(int bytes) {
    if (bytes < I2C_MIN_BYTES || bytes > I2C_MAX_BYTES) {
        return ErrorCode::I2C_INVALID_BYTE_COUNT;
    }
    return ErrorCode::SUCCESS;
}

ErrorCode ErrorHandler::validateSPIByteCount(int bytes) {
    if (bytes < SPI_MIN_BYTES || bytes > SPI_MAX_BYTES) {
        return ErrorCode::SPI_INVALID_BYTE_COUNT;
    }
    return ErrorCode::SUCCESS;
}

ErrorCode ErrorHandler::validateBaudrate(int baudrate) {
    int validBaudrates[] = {9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600};
    for (int i = 0; i < sizeof(validBaudrates) / sizeof(validBaudrates[0]); i++) {
        if (baudrate == validBaudrates[i]) {
            return ErrorCode::SUCCESS;
        }
    }
    return ErrorCode::UART_INVALID_BAUDRATE;
}

ErrorCode ErrorHandler::validateSampleCount(int samples) {
    if (samples < 1 || samples > 1024) {
        return ErrorCode::INVALID_PARAMETER;
    }
    return ErrorCode::SUCCESS;
}
