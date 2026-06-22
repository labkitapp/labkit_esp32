#include "CommandHandlers.h"
#include "Config.h"
#include "I2CManager.h"
#include "PinManager.h"
#include "SPIManager.h"
#include <Arduino.h>
#include <WiFi.h>

// Forward declarations for external variables
extern AsyncWebSocket ws;
extern int current_uart_baudrate;
extern uint8_t spirecv[5];

// ---------------------------------------------------------------------------
// Hex string utilities
// ---------------------------------------------------------------------------

// Parse "0xAB" or "0xABCD" → byte array, big-endian. Returns byte count, -1 on error.
static int hexStringToBytes(const String& hex, uint8_t* buf, size_t bufSize) {
    String s = hex;
    s.trim();
    if (s.startsWith("0x") || s.startsWith("0X")) s = s.substring(2);
    if (s.length() == 0 || s.length() % 2 != 0) return -1;
    size_t byteCount = s.length() / 2;
    if (byteCount > bufSize) return -1;
    for (size_t i = 0; i < byteCount; i++) {
        char hi = s[i * 2], lo = s[i * 2 + 1];
        auto hexVal = [](char c) -> int {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            return -1;
        };
        int h = hexVal(hi), l = hexVal(lo);
        if (h < 0 || l < 0) return -1;
        buf[i] = (uint8_t)((h << 4) | l);
    }
    return (int)byteCount;
}

// Produce "0xABCD" from byte array, big-endian.
static String bytesToHexString(const uint8_t* buf, size_t len) {
    String s = "0x";
    for (size_t i = 0; i < len; i++) {
        if (buf[i] < 0x10) s += "0";
        s += String(buf[i], HEX);
    }
    return s;
}

static void copyRequestIdIfPresent(const JsonDocument& doc, JsonDocument& out) {
    if (!doc.containsKey("request_id")) return;
    if (doc["request_id"].isNull()) return;
    if (doc["request_id"].is<const char*>()) {
        out["request_id"] = doc["request_id"].as<const char*>();
    } else {
        out["request_id"] = doc["request_id"].as<int32_t>();
    }
}

// ---------------------------------------------------------------------------
// I2C Command Handlers
// ---------------------------------------------------------------------------

void handleI2CTransactionCommand(const JsonDocument &doc) {
#if LABKIT_I2C_TIMING_LOG
    const uint32_t t0 = micros();
#endif
    StaticJsonDocument<512> out;
    I2CManager::transaction(doc, out);
#if LABKIT_I2C_TIMING_LOG
    const uint32_t t1 = micros();
#endif
    String response;
    serializeJson(out, response);
#if LABKIT_I2C_TIMING_LOG
    const uint32_t t2 = micros();
#endif
    ws.textAll(response);
#if LABKIT_I2C_TIMING_LOG
    const uint32_t t3 = micros();
    I2C_TIMING_PRINTF(
        "[I2C-TIMING] handler transaction_us=%lu serialize_us=%lu textAll_us=%lu total_us=%lu\n",
        (unsigned long)(t1 - t0), (unsigned long)(t2 - t1), (unsigned long)(t3 - t2),
        (unsigned long)(t3 - t0));
#endif
}

void handleI2CSequenceCommand(const JsonDocument &doc) {
    StaticJsonDocument<1024> out;
    I2CManager::sequence(doc, out);
    String response;
    serializeJson(out, response);
    ws.textAll(response);
}

void handleI2CRMWCommand(const JsonDocument &doc) {
    StaticJsonDocument<256> out;
    I2CManager::rmw(doc, out);
    String response;
    serializeJson(out, response);
    ws.textAll(response);
}

// I2C Scan Command Handler
void handleI2CScanCommand(const JsonDocument &doc) {
  String devices = I2CManager::scan();
  String response = JsonResponseBuilder::createI2CScanResponse(devices);
  ws.textAll(response);
}

// I2C Bus Check Command Handler
// Three-phase test:
//   Phase 1 — float pins, read: HIGH = external pull-up present
//   Phase 2 — drive pins LOW as output, read: LOW = GPIO can sink the line
//             (a HIGH here means a near-zero-ohm short to VCC)
//   Phase 3 — release back to floating input, read: HIGH = bus is free
//             (a LOW here means a device or short is holding the line down)
void handleI2CBusCheckCommand(const JsonDocument &doc) {
  Wire.end();

  // Phase 1: pull-up detection
  pinMode(I2C_SCL_PIN, INPUT);
  pinMode(I2C_SDA_PIN, INPUT);
  delay(5);
  bool scl_high = digitalRead(I2C_SCL_PIN);
  bool sda_high = digitalRead(I2C_SDA_PIN);

  // Phase 2: drive low — test that the GPIO can actually pull the line to GND
  pinMode(I2C_SCL_PIN, OUTPUT);
  digitalWrite(I2C_SCL_PIN, LOW);
  pinMode(I2C_SDA_PIN, OUTPUT);
  digitalWrite(I2C_SDA_PIN, LOW);
  delay(2);
  bool scl_pullable_low = (digitalRead(I2C_SCL_PIN) == LOW);
  bool sda_pullable_low = (digitalRead(I2C_SDA_PIN) == LOW);

  // Phase 3: release and check recovery — detects bus stuck low
  pinMode(I2C_SCL_PIN, INPUT);
  pinMode(I2C_SDA_PIN, INPUT);
  delay(5);
  bool scl_free = digitalRead(I2C_SCL_PIN);
  bool sda_free = digitalRead(I2C_SDA_PIN);

  // Restore I2C
  I2CManager::initialize(I2C_SDA_PIN, I2C_SCL_PIN, 100000);

  String response = JsonResponseBuilder::createI2CBusCheckResponse(
      I2C_SCL_PIN, I2C_SDA_PIN,
      scl_high, sda_high,
      scl_pullable_low, sda_pullable_low,
      scl_free, sda_free);
  ws.textAll(response);
}

// Read All Registers Command Handler
void handleReadAllRegistersCommand(const JsonDocument &doc) {
  int devAddress = doc["dev"];
  int regWidth = doc["width"];

  ErrorCode err = ErrorHandler::validateDeviceAddress(devAddress);
  if (err != ErrorCode::SUCCESS) {
    ErrorHandler::sendError(err, "Invalid device address: " + String(devAddress), __FUNCTION__);
    return;
  }

  StaticJsonDocument<2048> response;
  response["function"] = "i2c_registers";
  response["dev"] = devAddress;
  response["width"] = regWidth;

  JsonObject registers = response.createNestedObject("regs");
  for (uint8_t reg = 0; reg < 128; reg++) {
    Wire.beginTransmission((uint8_t)devAddress);
    Wire.write(reg);
    if (Wire.endTransmission(false) == 0) {
      uint8_t got = Wire.requestFrom((uint8_t)devAddress, (uint8_t)regWidth);
      if (got >= regWidth) {
        uint32_t regval = 0;
        for (int b = 0; b < regWidth; b++) {
          regval = (regval << 8) | Wire.read();
        }
        registers[String(reg)] = regval;
      }
    }
  }

  String jsonResponse;
  serializeJson(response, jsonResponse);
  ws.textAll(jsonResponse);
}

// ---------------------------------------------------------------------------
// SPI Command Handler
// ---------------------------------------------------------------------------

void handleSPITransactionCommand(const JsonDocument &doc) {
    StaticJsonDocument<512> out;
    copyRequestIdIfPresent(doc, out);

    String txHex = doc["tx"] | "";
    if (txHex.isEmpty()) {
        out["function"] = "spi_transaction";
        out["error"] = "Missing tx";
        out["code"] = (int)ErrorCode::INVALID_PARAMETER;
        String response;
        serializeJson(out, response);
        ws.textAll(response);
        return;
    }

    uint8_t txBuf[SPI_MAX_BYTES];
    int txLen = hexStringToBytes(txHex, txBuf, SPI_MAX_BYTES);
    if (txLen < 1) {
        out["function"] = "spi_transaction";
        out["error"] = "Invalid tx hex";
        out["code"] = (int)ErrorCode::INVALID_PARAMETER;
        String response;
        serializeJson(out, response);
        ws.textAll(response);
        return;
    }

    uint8_t rxBuf[SPI_RECEIVE_BUFFER_SIZE] = {0};
    ErrorCode result = SPIManager::command(txBuf, (uint8_t)txLen, rxBuf);
    if (result != ErrorCode::SUCCESS) {
        out["function"] = "spi_transaction";
        out["error"] = "SPI transaction failed";
        out["code"] = (int)result;
        String response;
        serializeJson(out, response);
        ws.textAll(response);
        return;
    }

    ws.textAll(JsonResponseBuilder::createSPITransactionResponse(bytesToHexString(rxBuf, txLen), doc));
}

// ---------------------------------------------------------------------------
// UART Command Handlers
// ---------------------------------------------------------------------------

void handleUARTBaudrateCommand(const JsonDocument &doc) {
  int baudrate = doc.containsKey("baudrate") ? doc["baudrate"] : 115200;

  ErrorCode err = ErrorHandler::validateBaudrate(baudrate);
  if (err != ErrorCode::SUCCESS) {
    ErrorHandler::sendError(err, "Invalid baudrate: " + String(baudrate), __FUNCTION__);
    return;
  }

  current_uart_baudrate = baudrate;
  Serial1.updateBaudRate(baudrate);

  ws.textAll(JsonResponseBuilder::createUARTBaudrateResponse(baudrate));
  DEBUG_PRINTF("DEBUG: UART baudrate set to %d\n", baudrate);
}

void handleUARTCommand(const JsonDocument &doc) {
    String data       = doc["data"]        | "";
    String encoding   = doc["encoding"]    | "ascii";
    String lineEnding = doc["line_ending"] | "CRLF";

    if (encoding == "hex") {
        String hex = data;
        hex.replace(" ", "");
        uint8_t buf[64];
        int len = hexStringToBytes("0x" + hex, buf, sizeof(buf));
        if (len < 0) {
            ErrorHandler::sendError(ErrorCode::INVALID_PARAMETER, "Invalid hex data", __FUNCTION__);
            return;
        }
        for (int i = 0; i < len; i++) Serial1.write(buf[i]);
    } else {
        Serial1.print(data);
    }

    if (lineEnding == "LF") {
        Serial1.print("\n");
    } else {
        Serial1.print("\r\n");
    }

    StaticJsonDocument<128> resp;
    resp["function"] = "uart";
    resp["sent"] = data;
    String out;
    serializeJson(resp, out);
    ws.textAll(out);
}

// ---------------------------------------------------------------------------
// GPIO Command Handlers
// ---------------------------------------------------------------------------

void handleGPIOOutCommand(const JsonDocument &doc) {
  int pinNum = doc.containsKey("pin") ? doc["pin"] : -1;
  int value = doc.containsKey("value")
                  ? doc["value"]
                  : (doc.containsKey("state") ? doc["state"] : 0);

  ErrorCode err = ErrorHandler::validatePinNumber(pinNum);
  if (err != ErrorCode::SUCCESS) {
    ErrorHandler::sendError(err, "Invalid pin number: " + String(pinNum), __FUNCTION__);
    return;
  }

  ErrorCode result = PinManager::writeDigital(pinNum, value);
  if (result != ErrorCode::SUCCESS) {
    ErrorHandler::sendError(result, "GPIO write failed", __FUNCTION__);
    return;
  }

  ws.textAll(JsonResponseBuilder::createGPIOOutResponse(pinNum, value));
}

void handleGPIOInCommand(const JsonDocument &doc) {
  int pinNum = doc.containsKey("pin") ? doc["pin"] : -1;

  ErrorCode err = ErrorHandler::validatePinNumber(pinNum);
  if (err != ErrorCode::SUCCESS) {
    ErrorHandler::sendError(err, "Invalid pin number: " + String(pinNum), __FUNCTION__);
    return;
  }

  int value;
  ErrorCode result = PinManager::readDigital(pinNum, &value);
  if (result != ErrorCode::SUCCESS) {
    ErrorHandler::sendError(result, "GPIO read failed", __FUNCTION__);
    return;
  }

  ws.textAll(JsonResponseBuilder::createGPIOInResponse(pinNum, value));
}

void handleSetPinCommand(const JsonDocument &doc) {
  int pinNum = doc.containsKey("pin") ? doc["pin"] : -1;
  int mode = -1;

  ErrorCode err = ErrorHandler::validatePinNumber(pinNum);
  if (err != ErrorCode::SUCCESS) {
    ErrorHandler::sendError(err, "Invalid pin number: " + String(pinNum), __FUNCTION__);
    return;
  }

  if (doc.containsKey("mode")) {
    if (doc["mode"].is<int>()) {
      mode = doc["mode"];
    } else if (doc["mode"].is<const char *>()) {
      String modeStr = doc["mode"].as<const char *>();
      if (modeStr.equals("output") || modeStr.equals("1")) {
        mode = 0;
      } else if (modeStr.equals("input") || modeStr.equals("0")) {
        mode = 1;
      } else if (modeStr.equals("highz") || modeStr.equals("2")) {
        mode = 2;
      }
    }
  }

  err = ErrorHandler::validatePinMode(mode);
  if (err != ErrorCode::SUCCESS) {
    ErrorHandler::sendError(err, "Invalid pin mode: " + String(mode), __FUNCTION__);
    return;
  }

  PinManager::PinMode pinMode;
  switch (mode) {
  case 0: pinMode = PinManager::PinMode::PIN_OUTPUT; break;
  case 1: pinMode = PinManager::PinMode::PIN_INPUT; break;
  case 2: pinMode = PinManager::PinMode::PIN_HIGH_Z; break;
  default:
    ErrorHandler::sendError(ErrorCode::INVALID_PARAMETER, "Invalid pin mode", __FUNCTION__);
    return;
  }

  ErrorCode result = PinManager::setPinMode(pinNum, pinMode);
  if (result != ErrorCode::SUCCESS) {
    ErrorHandler::sendError(result, "Pin configuration failed", __FUNCTION__);
    return;
  }

  String modeStr;
  switch (mode) {
  case 0: modeStr = "output"; break;
  case 1: modeStr = "input"; break;
  case 2: modeStr = "highz"; break;
  default: modeStr = "unknown"; break;
  }

  ws.textAll(JsonResponseBuilder::createSetPinResponse(pinNum, modeStr));
}

// ---------------------------------------------------------------------------
// System Command Handlers
// ---------------------------------------------------------------------------

void handleStatusCommand(const JsonDocument &doc) {
  String ssid = "-";
  String ip = "-";

  if (WiFi.status() == WL_CONNECTED) {
    ssid = WiFi.SSID();
    ip = WiFi.localIP().toString();
  }

  static int pinModes[MAX_PIN_ID];
  static int pinValues[MAX_PIN_ID];

  for (size_t i = 0; i < MAX_PIN_ID; i++) {
    pinModes[i] = PinManager::getPinModeValue(i);
    pinValues[i] = PinManager::getPinValue(i);
  }

  ws.textAll(JsonResponseBuilder::createStatusResponse(
      ssid, ip, current_uart_baudrate, DEVICE_FEATURES, pinModes, pinValues, MAX_PIN_ID));
}

void handleReadAnalogCommand(const JsonDocument &doc) {
  int samples = doc["samples"];

  ErrorCode err = ErrorHandler::validateSampleCount(samples);
  if (err != ErrorCode::SUCCESS) {
    ErrorHandler::sendError(err, "Invalid sample count: " + String(samples), __FUNCTION__);
    return;
  }

  int analogPin = PinManager::getAnalogPin();
  static uint16_t analogBuffer[1024];

  analogBuffer[0] = 0xaaaa;
  for (int i = 1; i < samples; i++) {
    int value;
    ErrorCode readErr = PinManager::readAnalog(analogPin, &value);
    if (readErr != ErrorCode::SUCCESS) {
      ErrorHandler::sendError(readErr, "Failed to read analog pin " + String(analogPin), __FUNCTION__);
      return;
    }
    analogBuffer[i] = value;
  }

  ws.binaryAll(reinterpret_cast<const char *>(analogBuffer), 2 * samples);
}

void handleWiFiCommand(const JsonDocument &doc) {
  String ssid = "";
  String password = "";

  if (doc.containsKey("ssid")) ssid = doc["ssid"].as<const char *>();
  if (doc.containsKey("password")) password = doc["password"].as<const char *>();

  if (ssid.length() == 0 || password.length() == 0) {
    ErrorHandler::sendError(ErrorCode::WIFI_CREDENTIALS_MISSING, "Missing SSID or password", __FUNCTION__);
    return;
  }

  WiFi.disconnect();
  if (connectToWiFi(ssid.c_str(), password.c_str())) {
    ws.textAll(JsonResponseBuilder::createWiFiResponse("WiFi connection initiated", ssid));
  } else {
    ErrorHandler::sendError(ErrorCode::WIFI_CONNECTION_FAILED, "Failed to connect to WiFi: " + ssid, __FUNCTION__);
  }
}

void handleClearCommand(const JsonDocument &doc) {
  clearWiFiCredentials();
  ws.textAll(JsonResponseBuilder::createClearResponse());
}

void handleArmCommand(const JsonDocument &doc) {
  int value = doc.containsKey("value") ? doc["value"] : 0;

  if (value != 0 && value != 1) {
    ErrorHandler::sendError(ErrorCode::INVALID_PARAMETER, "Invalid arm value: " + String(value), __FUNCTION__);
    return;
  }

  DEBUG_PRINTF("DEBUG: Arm command executed with value %d\n", value);
  ws.textAll(JsonResponseBuilder::createArmResponse(value));
}

// ---------------------------------------------------------------------------
// Pin State Monitoring (called by main loop)
// ---------------------------------------------------------------------------

void checkAndNotifyPinChanges() {
  static int changedPins[MAX_PIN_ID];
  static int changedValues[MAX_PIN_ID];
  int changeCount = PinManager::checkPinStates(changedPins, changedValues);

  if (changeCount > 0) {
    String jsonResponse = JsonResponseBuilder::createPinStateChangeResponse(changedPins, changedValues, changeCount);
    ws.textAll(jsonResponse);

#if DEBUG_ENABLED
    DEBUG_PRINTLN("DEBUG: Sending pin state change: " + jsonResponse);
#endif
  }
}
