#ifndef JSON_RESPONSE_BUILDER_H
#define JSON_RESPONSE_BUILDER_H

#include <ArduinoJson.h>
#include "ErrorHandler.h"

class JsonResponseBuilder {
public:
    // I2C responses
    static String createI2CReadResponse(const uint8_t* rx, size_t rxLen, int regWidth);
    static String createI2CWriteResponse(const String& dev, const String& reg, const String& tx);
    static String createI2CScanResponse(const String& devices);
    static String createI2CBusCheckResponse(int scl_pin, int sda_pin,
                                             bool scl_high, bool sda_high,
                                             bool scl_pullable_low, bool sda_pullable_low,
                                             bool scl_free, bool sda_free);

    // SPI response (optional request_id echoed from client command doc)
    static String createSPITransactionResponse(const String& rx, const JsonDocument& docIn);

    // UART response
    static String createUARTBaudrateResponse(int baudrate);

    // GPIO responses
    static String createGPIOOutResponse(int pin, int value);
    static String createGPIOInResponse(int pin, int value);
    static String createSetPinResponse(int pin, const String& mode);

    // System responses
    static String createStatusResponse(const String& ssid, const String& ip, int uartBaudrate,
                                       const String& features, const int* pinModes, const int* pinValues, int pinCount);
    static String createWiFiResponse(const String& message, const String& ssid = "");
    static String createClearResponse();
    static String createArmResponse(int value);

    // Pin state change response
    static String createPinStateChangeResponse(const int* changedPins, const int* changedValues, int changeCount);

    // Error responses (delegated to ErrorHandler)
    static String createErrorResponse(ErrorCode code, const String& message,
                                      const String& function = "", const JsonObject& additionalData = JsonObject());

private:
    static StaticJsonDocument<512> createBaseDocument(const String& function);
    static String serializeDocument(const StaticJsonDocument<512>& doc);
};

#endif // JSON_RESPONSE_BUILDER_H
