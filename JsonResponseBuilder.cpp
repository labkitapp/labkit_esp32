#include "Config.h"
#include "JsonResponseBuilder.h"
#include "ErrorHandler.h"
#include <Arduino.h>

String JsonResponseBuilder::createI2CReadResponse(const uint8_t* rx, size_t rxLen, int regWidth) {
    StaticJsonDocument<256> doc;
    doc["function"] = "i2c_read";
    String rxHex = "0x";
    for (size_t i = 0; i < rxLen; i++) {
        if (rx[i] < 0x10) rxHex += "0";
        rxHex += String(rx[i], HEX);
    }
    doc["rx"] = rxHex;
    doc["reg_width"] = regWidth;
    String out;
    serializeJson(doc, out);
    return out;
}

String JsonResponseBuilder::createI2CWriteResponse(const String& dev, const String& reg, const String& tx) {
    StaticJsonDocument<256> doc;
    doc["function"] = "i2c_write";
    doc["dev"] = dev;
    doc["reg"] = reg;
    doc["tx"] = tx;
    String out;
    serializeJson(doc, out);
    return out;
}

String JsonResponseBuilder::createI2CScanResponse(const String& devices) {
    StaticJsonDocument<512> doc = createBaseDocument("i2c_scan");

    JsonArray deviceArray = doc.createNestedArray("devices");
    if (devices.length() > 0) {
        int start = 0;
        int end = devices.indexOf(',');
        while (end != -1) {
            String device = devices.substring(start, end);
            device.trim();
            deviceArray.add(device.startsWith("0x") ? device : "0x" + device);
            start = end + 1;
            end = devices.indexOf(',', start);
        }
        String lastDevice = devices.substring(start);
        lastDevice.trim();
        if (lastDevice.length() > 0) {
            deviceArray.add(lastDevice.startsWith("0x") ? lastDevice : "0x" + lastDevice);
        }
    }

    return serializeDocument(doc);
}

String JsonResponseBuilder::createI2CBusCheckResponse(
    int scl_pin, int sda_pin,
    bool scl_high, bool sda_high,
    bool scl_pullable_low, bool sda_pullable_low,
    bool scl_free, bool sda_free) {
    StaticJsonDocument<512> doc = createBaseDocument("i2c_bus_check");
    doc["scl_pin"] = scl_pin;
    doc["sda_pin"] = sda_pin;
    doc["scl_high"] = scl_high;
    doc["sda_high"] = sda_high;
    doc["scl_pullable_low"] = scl_pullable_low;
    doc["sda_pullable_low"] = sda_pullable_low;
    doc["scl_free"] = scl_free;
    doc["sda_free"] = sda_free;
    doc["pullups_ok"] = scl_high && sda_high;
    doc["bus_ok"] = scl_high && sda_high &&
                    scl_pullable_low && sda_pullable_low &&
                    scl_free && sda_free;
    return serializeDocument(doc);
}

String JsonResponseBuilder::createSPITransactionResponse(const String& rx, const JsonDocument& docIn) {
    StaticJsonDocument<256> doc;
    doc["function"] = "spi_transaction";
    doc["rx"] = rx;
    if (docIn.containsKey("request_id") && !docIn["request_id"].isNull()) {
        if (docIn["request_id"].is<const char*>()) {
            doc["request_id"] = docIn["request_id"].as<const char*>();
        } else {
            doc["request_id"] = docIn["request_id"].as<int32_t>();
        }
    }
    String out;
    serializeJson(doc, out);
    return out;
}

String JsonResponseBuilder::createUARTBaudrateResponse(int baudrate) {
    StaticJsonDocument<512> doc = createBaseDocument("uart_baudrate");
    doc["message"] = "UART baudrate updated successfully";
    doc["baudrate"] = baudrate;
    return serializeDocument(doc);
}

String JsonResponseBuilder::createGPIOOutResponse(int pin, int value) {
    StaticJsonDocument<512> doc = createBaseDocument("gpio_out");
    doc["pin"] = pin;
    doc["value"] = value;
    return serializeDocument(doc);
}

String JsonResponseBuilder::createGPIOInResponse(int pin, int value) {
    StaticJsonDocument<512> doc = createBaseDocument("gpio_in");
    doc["pin"] = pin;
    doc["value"] = value;
    return serializeDocument(doc);
}

String JsonResponseBuilder::createSetPinResponse(int pin, const String& mode) {
    StaticJsonDocument<512> doc = createBaseDocument("set_pin");
    doc["pin"] = pin;
    doc["mode"] = mode;
    return serializeDocument(doc);
}

String JsonResponseBuilder::createStatusResponse(const String& ssid, const String& ip, int uartBaudrate,
                                                const String& features, const int* pinModes, const int* pinValues, int pinCount) {
    StaticJsonDocument<1024> doc;
    doc["function"] = "status";
    doc["labkit"] = DEVICE_NAME;
    doc["version"] = FIRMWARE_VERSION;
    doc["date"] = BUILD_INFO_STRING;

    JsonObject status = doc.createNestedObject("status");
    status["ssid"] = ssid;
    status["ip"] = ip;
    status["uart_baudrate"] = uartBaudrate;

    doc["features"] = features;

    JsonObject pins = doc.createNestedObject("pins");
    JsonArray inputPins = pins.createNestedArray("input");
    JsonArray outputPins = pins.createNestedArray("output");
    JsonArray highzPins = pins.createNestedArray("highz");

    for (int i = 0; i < pinCount; i++) {
        int mode = pinModes[i];
        int val  = pinValues[i];
        if (mode == 1) {
            JsonObject p = inputPins.createNestedObject();
            p["pin"] = i; p["current_state"] = val;
        } else if (mode == 0) {
            JsonObject p = outputPins.createNestedObject();
            p["pin"] = i; p["current_state"] = val;
        } else if (mode == 2) {
            JsonObject p = highzPins.createNestedObject();
            p["pin"] = i; p["current_state"] = val;
        }
    }

    String response;
    serializeJson(doc, response);
    return response;
}

String JsonResponseBuilder::createWiFiResponse(const String& message, const String& ssid) {
    StaticJsonDocument<512> doc = createBaseDocument("wifi");
    doc["message"] = message;
    if (ssid.length() > 0) doc["ssid"] = ssid;
    return serializeDocument(doc);
}

String JsonResponseBuilder::createClearResponse() {
    StaticJsonDocument<512> doc = createBaseDocument("clear");
    doc["message"] = "WiFi credentials cleared";
    return serializeDocument(doc);
}

String JsonResponseBuilder::createArmResponse(int value) {
    StaticJsonDocument<512> doc = createBaseDocument("arm");
    doc["message"] = "Arm command executed";
    doc["value"] = value;
    return serializeDocument(doc);
}

String JsonResponseBuilder::createErrorResponse(ErrorCode code, const String& message,
                                              const String& function, const JsonObject& additionalData) {
    return ErrorHandler::createErrorResponse(code, message, function, additionalData);
}

String JsonResponseBuilder::createPinStateChangeResponse(const int* changedPins, const int* changedValues, int changeCount) {
    StaticJsonDocument<512> doc;
    doc["function"] = "pins";
    JsonObject data = doc.createNestedObject("states");
    for (int i = 0; i < changeCount; i++) {
        data[String(changedPins[i])] = changedValues[i];
    }
    String response;
    serializeJson(doc, response);
    return response;
}

StaticJsonDocument<512> JsonResponseBuilder::createBaseDocument(const String& function) {
    StaticJsonDocument<512> doc;
    doc["function"] = function;
    return doc;
}

String JsonResponseBuilder::serializeDocument(const StaticJsonDocument<512>& doc) {
    String response;
    serializeJson(doc, response);
    return response;
}
