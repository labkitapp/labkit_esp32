#include "CommandHandlers.h"
#include "Config.h"
#include "ErrorHandler.h"
#include "I2CManager.h"
#include "JsonResponseBuilder.h"
#include "PinManager.h"
#include "SPIManager.h"
#include "WebSocketCommandHandler.h"
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <WiFi.h>
#include <Wire.h>

#include "index_html.h"

#include "board_define.h"

#ifdef ESP32DEVKIT
#include <SPI.h>
#define DEVICE "labkit_esp32"
#define SCL_PIN 22
#define SDA_PIN 21
#define SERIAL1_RX 17
#define SERIAL1_TX 16
// SPI functionality moved to SPIManager class
#elif defined(LABKIT_ONE)
#define DEVICE "labkit_one"
// #define SCL_PIN 7
// #define SDA_PIN 6
#define SCL_PIN 4
#define SDA_PIN 5
#define ENABLE_WS_SERIAL 0
#define SERIAL1_RX 41
#define SERIAL1_TX 40
#endif

Preferences preferences;
// Pin management moved to PinManager class
unsigned long last_pin_check_time = 0;

AsyncWebServer server(HTTP_SERVER_PORT);
AsyncWebSocket ws("/ws");

// Constants now defined in Config.h

String uart_message = "";

// Pin management moved to PinManager class
bool ws_ready = false;

// UART baudrate configuration
int current_uart_baudrate = UART_DEFAULT_BAUDRATE;

// Prototypes of all functions

// Pin functions moved to PinManager class

// I2C functions moved to I2CManager class

// I2C functions moved to I2CManager class

// I2C functions moved to I2CManager class

// SPI functions moved to SPIManager class

// I2C functions moved to I2CManager class

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
             AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(),
                  client->remoteIP().toString().c_str());
    ws_ready = true;
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    ws_ready = false;
    break;
  case WS_EVT_DATA:
    WebSocketCommandHandler::handleMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String &var) {
  // Device information variables
  if (var == "DEVICE_NAME") {
    return DEVICE_NAME;
  }
  if (var == "FIRMWARE_VERSION") {
    return FIRMWARE_VERSION;
  }
  if (var == "BUILD_DATE") {
    return String(__DATE__);
  }
  if (var == "BUILD_TIME") {
    return String(__TIME__);
  }
  if (var == "WIFI_SSID") {
    if (WiFi.status() == WL_CONNECTED) {
      return WiFi.SSID();
    }
    return "Not Connected";
  }
  if (var == "IP_ADDRESS") {
    if (WiFi.status() == WL_CONNECTED) {
      return WiFi.localIP().toString();
    }
    return "0.0.0.0";
  }
  if (var == "MAC_ADDRESS") {
    return WiFi.macAddress();
  }
  if (var == "UPTIME") {
    return String(millis() / 1000);
  }
  if (var == "FREE_HEAP") {
    return String(ESP.getFreeHeap());
  }
  if (var == "MAX_ALLOC_HEAP") {
    return String(ESP.getMaxAllocHeap());
  }
  if (var == "DEVICE_FEATURES") {
    return DEVICE_FEATURES;
  }

  return String();
}

void startServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  server.begin();
}

bool connectToWiFi(const char *ssid, const char *password) {
  WiFi.begin(ssid, password);

  unsigned long startAttemptTime = millis();
  const unsigned long timeout = WIFI_CONNECTION_TIMEOUT_MS;

  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startAttemptTime >= timeout) {
      Serial.println("Failed to connect to WiFi within the timeout period.");
      return false;
    }
    delay(1000);
    Serial.printf("Connecting to %s...\n", ssid);
  }

  Serial.printf("\n!!! Make sure you are also connected to %s\n", ssid);

  String urlLine = "  Visit: http://" + WiFi.localIP().toString();
  while (urlLine.length() < 30) {
    urlLine += " ";
  }
  Serial.println(urlLine);

  Serial.println("     to get started          ");
  Serial.println("=========================");

  Serial.println();

  // Save the WiFi credentials to Preferences
  preferences.begin(WIFI_CREDENTIALS_NAMESPACE, false);
  preferences.putString("ssid", ssid);
  preferences.putString("password", password);
  preferences.end();

  initWebSocket();
  startServer();

  
  // Initialize Pin Manager
  PinManager::initialize();
  
  // Initialize I2C Manager
  I2CManager::initialize(SDA_PIN, SCL_PIN, 100000);

  // Initialize SPI Manager
  SPIManager::initialize(5, 1000000, SPI_MODE0); // SS pin 5, 1MHz, Mode 0


  // Initialize error handler and command handler
  ErrorHandler::initialize(&ws, DEBUG_ENABLED);
  WebSocketCommandHandler::initialize(&ws);

  return true;
}

void clearWiFiCredentials() {
  preferences.begin(WIFI_CREDENTIALS_NAMESPACE, false);
  preferences.clear();
  preferences.end();
  WiFi.disconnect();
  Serial.println("WiFi credentials cleared and disconnected.");
}

// Update the process_ws_command function to handle JSON commands properly
// bool process_ws_command() { ... } - REMOVED

void checkUART() {
  if (Serial.available() > 0) {
    uart_message = Serial.readStringUntil('\n');
    uart_message.trim();

    // Check for WiFi message format: wifi ssid password
    if (uart_message.startsWith("wifi ")) {
      // Extract SSID and password from wifi ssid password format
      String content = uart_message.substring(5); // Remove "wifi "

      // Find the first space separator (between SSID and password)
      int spaceIndex = content.indexOf(' ');
      if (spaceIndex > 0) {
        String ssid = content.substring(0, spaceIndex);
        String password = content.substring(spaceIndex + 1);

        DEBUG_PRINTF(
            "DEBUG: Parsed WiFi credentials - SSID: %s, Password: %s\n",
            ssid.c_str(), password.c_str());

        // Connect to WiFi
        WiFi.disconnect();
        Serial.println("Attempting WiFi connection to: " + ssid);
        if (connectToWiFi(ssid.c_str(), password.c_str())) {
          Serial.println("WiFi connection successful via UART command");
        } else {
          Serial.println("WiFi connection failed via UART command");
        }
      } else {
        Serial.println("Invalid WiFi message format. Expected: wifi ssid "
                       "password OR scan");
      }
    } else if (uart_message.equals("scan")) {
      // Handle WiFi scan command
      Serial.println("Scanning for WiFi networks...");
      int numNetworks = WiFi.scanNetworks();

      if (numNetworks == 0) {
        Serial.println("No WiFi networks found");
      } else {
        Serial.printf("Found %d networks:\n", numNetworks);
        for (int i = 0; i < numNetworks; i++) {
          Serial.printf(
              "%d: %s (%s) - RSSI: %d dBm\n", i + 1, WiFi.SSID(i).c_str(),
              WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "Open" : "Secured",
              WiFi.RSSI(i));
        }
      }

      // Handle JSON commands
    } else {
      Serial.println("Supported commands: wifi ssid password OR scan");
    }
  }
}

#if ENABLE_WS_SERIAL == 1
static uint8_t uartBuffer[UART_BUFFER_SIZE];
static int bufferIndex = 0;

void checkUART1() {
  if (Serial1.available() > 0) {
    uint8_t byte = Serial1.read();

    // Line terminator: send buffered line over WebSocket (do not treat as invalid)
    if (byte == '\n' || byte == '\r' || bufferIndex >= sizeof(uartBuffer) - 1) {
      // Only send if we actually have content; this avoids blank lines
      if (bufferIndex > 0) {
        uartBuffer[bufferIndex] = '\0';

        StaticJsonDocument<256> doc;
        doc["function"] = "uart";
        doc["data"] = reinterpret_cast<const char *>(uartBuffer);

        String jsonResponse;
        serializeJson(doc, jsonResponse);
        ws.textAll(jsonResponse);
      }
      bufferIndex = 0;
      return;
    }

    // Reject other non-printable characters
    if (byte < 32 || byte > 126) {
      bufferIndex = 0;
      Serial1.flush();
      return;
    }

    uartBuffer[bufferIndex++] = byte;
  }
}

#endif

void setup() {
#ifdef ESP32DEVKIT
  Serial.begin(115200);
  Serial.println(" ------ Labkit ESP32 v0.0 ------");
#elif defined(LABKIT_ONE)
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, SERIAL1_RX, SERIAL1_TX); //
  Serial1.println(" ------ Labkit ONE v0.0 ------");
#endif

  Serial.printf("Compiled on %s  %s\n", __DATE__, __TIME__);

  Serial.println();
  Serial.println("=========================");
  Serial.println(" ╔═════════════════════╗");
  Serial.println(" ║  ██╗      █████╗ ██████╗");
  Serial.println(" ║  ██║     ██╔══██╗██╔══██╗");
  Serial.println(" ║  ██║     ███████║██████╔╝");
  Serial.println(" ║  ██║     ██╔══██║██╔══██╗");
  Serial.println(" ║  ███████╗██║  ██║██████╔╝");
  Serial.println(" ║  ╚══════╝╚═╝  ╚═╝╚═════╝ ");
  Serial.println(" ║                     ");
  Serial.println(" ║  ██╗  ██╗██████╗████████╗");
  Serial.println(" ║  ██║ ██╔╝╚═██╔═╝╚══██╔══╝");
  Serial.println(" ║  █████╔╝   ██║     ██║   ");
  Serial.println(" ║  ██╔═██╗   ██║     ██║   ");
  Serial.println(" ║  ██║  ██╗██████╗   ██║   ");
  Serial.println(" ║  ╚═╝  ╚═╝╚═════╝   ╚═╝   ");
  Serial.println(" ║                         ");
  Serial.println(" ║ 🚀 ESP32 HARDWARE   ");
  Serial.println(" ║      DEBUGGER READY    ");
  Serial.println(" ╚═══════════════════════╝");
  Serial.println("=========================");

  // I2C initialization is handled by I2CManager when WiFi connects

#ifdef ESP32DEVKIT
  // SPI initialization moved to SPIManager
#endif

#if ENABLE_WS_SERIAL == 1
#ifdef ESP32DEVKIT
  Serial.printf("Enabling Serial1 on [%d,%d] at %d baud\n", SERIAL1_RX,
                SERIAL1_TX, current_uart_baudrate);
  Serial1.begin(current_uart_baudrate, SERIAL_8N1, SERIAL1_RX, SERIAL1_TX);
#elif defined(LABKIT_ONE)
  Serial.printf("Enabling Serial1 on [%d,%d] at %d baud\n", SERIAL1_RX,
                SERIAL1_TX, current_uart_baudrate);
  Serial1.begin(current_uart_baudrate, SERIAL_8N1, SERIAL1_RX, SERIAL1_TX);
#endif
#endif

  // Load WiFi credentials from Preferences
  preferences.begin(WIFI_CREDENTIALS_NAMESPACE, true);
  String storedSSID = preferences.getString("ssid", "");
  String storedPassword = preferences.getString("password", "");
  preferences.end();

  if (storedSSID.length() > 0 && storedPassword.length() > 0) {
    if (!connectToWiFi(storedSSID.c_str(), storedPassword.c_str())) {
      Serial.printf("Cannot Connect to WiFi %s\n", storedSSID.c_str());
      Serial.println("WiFi not connected. To connect, use the command:");
      Serial.println("wifi ssid password");
      Serial.println("Example: wifi MyNetwork MyPassword");
      Serial.println("To scan for available networks, use: scan");
      Serial.println();
    }
  } else {
    Serial.println("No WiFi credentials found. To connect, use the command:");
    Serial.println("wifi ssid password");
    Serial.println("Example: wifi MyNetwork MyPassword");
    Serial.println("To scan for available networks, use: scan");
    Serial.println();
  }

  if (MDNS.begin(MDNS_HOSTNAME)) {
    MDNS.addService("http", MDNS_SERVICE_PROTOCOL, MDNS_SERVICE_PORT);
    MDNS.addService(MDNS_SERVICE_NAME, MDNS_SERVICE_PROTOCOL,
                    MDNS_SERVICE_PORT);
    Serial.println("mDNS service 'labkit' added");
  }

  PinManager::initializeAllPinsToHighZ();
}

bool check_timeout(unsigned long timer,
                   uint32_t timeout = PIN_STATE_CHECK_TIMEOUT_MS) {
  if (millis() - timer > timeout) {
    return true;
  }
  return false;
}
// Pin functions moved to PinManager class

// Check WiFi status and print command format if not connected
void checkWiFiStatus() {
  static unsigned long lastWiFiCheck = 0;
  static bool lastWiFiStatus = false;
  const unsigned long WIFI_CHECK_INTERVAL = WIFI_CHECK_INTERVAL_MS;

  // Only check periodically to avoid spam
  if (millis() - lastWiFiCheck >= WIFI_CHECK_INTERVAL) {
    bool currentWiFiStatus = (WiFi.status() == WL_CONNECTED);

    // If WiFi status changed from connected to disconnected, or if disconnected
    // for the first time
    if ((lastWiFiStatus && !currentWiFiStatus) ||
        (!lastWiFiStatus && !currentWiFiStatus && lastWiFiCheck > 0)) {
      Serial.print("WiFi not connected. To connect, use the command: wifi ssid "
                   "password");
      Serial.println();
    } else if (currentWiFiStatus && !ws_ready) {
      Serial.println("Visit: http://" + WiFi.localIP().toString() +
                     " to get started.");
    }

    lastWiFiStatus = currentWiFiStatus;
    lastWiFiCheck = millis();
  }
}

void loop() {
  ws.cleanupClients();

  checkUART();

#if ENABLE_WS_SERIAL == 1
  checkUART1();
#endif

  // Check WiFi status and print command format if needed
  checkWiFiStatus();

  // Check for pin state changes and notify clients (scheduled by main loop)
  if (ws_ready && check_timeout(last_pin_check_time, GPIO_CHECK_INTERVAL_MS)) {
    checkAndNotifyPinChanges();
    last_pin_check_time = millis();
  }
}
