#ifndef CONFIG_H
#define CONFIG_H

#include "board_define.h"
// =============================================================================
// SYSTEM CONFIGURATION
// =============================================================================

// Debug Configuration
#define DEBUG_ENABLED 0
#define DEBUG_LEVEL_ERROR 1
#define DEBUG_LEVEL_WARNING 2
#define DEBUG_LEVEL_INFO 3
#define DEBUG_LEVEL_VERBOSE 4

// Current debug level
#define DEBUG_LEVEL DEBUG_LEVEL_INFO

// Debug macros
#if DEBUG_ENABLED
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINTF(fmt, ...) Serial.printf(fmt, __VA_ARGS__)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTF(fmt, ...)
#endif

// Set to 1 to print [I2C-TIMING] lines on Serial for i2c_transaction latency
// debugging. Set to 0 for normal builds (no extra Serial traffic).
#ifndef LABKIT_I2C_TIMING_LOG
#define LABKIT_I2C_TIMING_LOG 1
#endif
#if LABKIT_I2C_TIMING_LOG
#define I2C_TIMING_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
#define I2C_TIMING_PRINTF(...) ((void)0)
#endif

// =============================================================================
// HARDWARE CONFIGURATION
// =============================================================================

// Pin Configuration
#define MAX_PIN_ID 35
#define ANALOG_PIN_DEFAULT A0

// GPIO Configuration
#define GPIO_CHECK_INTERVAL_MS 300
#define PIN_MODE_OUTPUT 0
#define PIN_MODE_INPUT 1
#define PIN_MODE_HIGHZ 2

// I2C Pin Configuration (board-specific)
#ifdef ESP32DEVKIT
#define I2C_SCL_PIN 22
#define I2C_SDA_PIN 21
#elif defined(LABKIT_ONE)
#define I2C_SCL_PIN 4
#define I2C_SDA_PIN 5
#else
#define I2C_SCL_PIN 22
#define I2C_SDA_PIN 21
#endif

// I2C Configuration
#define I2C_TIMEOUT_MS 100
#define I2C_SCAN_START_ADDR 1
#define I2C_SCAN_END_ADDR 127
#define I2C_MAX_BYTES 4
#define I2C_MIN_BYTES 1
#define I2C_REGISTER_SCAN_MAX 128

// SPI Configuration
#define SPI_FREQUENCY_HZ 1000000
#define SPI_MAX_BYTES 8
#define SPI_MIN_BYTES 1

// UART Configuration
#define UART_DEFAULT_BAUDRATE 115200
#define UART_MAX_BUFFER_SIZE 512
#define UART_VALID_BAUDRATES                                                   \
  {9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600}
#define UART_VALID_BAUDRATES_COUNT 8

// Analog Configuration
#define ANALOG_BUFFER_SIZE 1024
#define ANALOG_MAX_SAMPLES 1024
#define ANALOG_MIN_SAMPLES 1
#define ANALOG_SYNC_WORD 0xAAAA

// =============================================================================
// NETWORK CONFIGURATION
// =============================================================================

// WiFi Configuration
#define WIFI_CONNECTION_TIMEOUT_MS 10000
#define WIFI_CHECK_INTERVAL_MS 3000
#define WIFI_CREDENTIALS_NAMESPACE "wifi"

// WebSocket Configuration
#define WEBSOCKET_MAX_MESSAGE_SIZE 512
#define WEBSOCKET_CLEANUP_INTERVAL_MS 1000

// HTTP Server Configuration
#define HTTP_SERVER_PORT 80
#define HTTP_MAX_REQUEST_SIZE 4096

// mDNS Configuration
#define MDNS_HOSTNAME "labkit-device"
#define MDNS_SERVICE_NAME "labkit"
#define MDNS_SERVICE_PROTOCOL "tcp"
#define MDNS_SERVICE_PORT 80

// =============================================================================
// JSON CONFIGURATION
// =============================================================================

// JSON Document Sizes
#define JSON_SMALL_DOCUMENT_SIZE 128
#define JSON_MEDIUM_DOCUMENT_SIZE 512
#define JSON_LARGE_DOCUMENT_SIZE 1024
#define JSON_EXTRA_LARGE_DOCUMENT_SIZE 2048

// JSON Response Types
#define JSON_RESPONSE_SUCCESS "success"
#define JSON_RESPONSE_ERROR "error"

// =============================================================================
// TIMING CONFIGURATION
// =============================================================================

// System Timing
#define SYSTEM_LOOP_DELAY_MS 1
#define PIN_STATE_CHECK_TIMEOUT_MS 20

// =============================================================================
// MEMORY CONFIGURATION
// =============================================================================

// Buffer Sizes
#define UART_BUFFER_SIZE 512
#define SPI_RECEIVE_BUFFER_SIZE SPI_MAX_BYTES

// =============================================================================
// DEVICE CONFIGURATION
// =============================================================================

// Version Information
#define FIRMWARE_VERSION "1.0"
#define BUILD_DATE __DATE__
#define BUILD_TIME __TIME__

// Device Names
#ifdef ESP32DEVKIT
#define DEVICE_NAME "labkit_esp32"
#define DEVICE_FEATURES "[\"i2c\",\"uart\",\"spi\",\"analog\"]"
#elif defined(LABKIT_ONE)
#define DEVICE_NAME "labkit_one"
#define DEVICE_FEATURES "[\"i2c\",\"rgb\",\"uart\",\"spi\",\"logic\"]"
#else
#define DEVICE_NAME "labkit_esp32"
#define DEVICE_FEATURES "[\"i2c\",\"uart\",\"spi\"]"
#endif

// =============================================================================
// ERROR CONFIGURATION
// =============================================================================

// Error Response Configuration
#define ERROR_MESSAGE_MAX_LENGTH 256
#define ERROR_DETAILS_MAX_LENGTH 512

// =============================================================================
// VALIDATION CONFIGURATION
// =============================================================================

// Parameter Validation
#define MIN_PIN_NUMBER 0
#define MAX_PIN_NUMBER (MAX_PIN_ID - 1)
#define MIN_DEVICE_ADDRESS 1
#define MAX_DEVICE_ADDRESS 127
#define MIN_REGISTER_ADDRESS 0
#define MAX_REGISTER_ADDRESS 255

// =============================================================================
// FEATURE FLAGS
// =============================================================================

// Optional Features
#define ENABLE_WS_SERIAL 1
#define ENABLE_ANALOG_READING 1
#define ENABLE_I2C_SCANNING 1
#define ENABLE_SPI_COMMUNICATION 1
#define ENABLE_GPIO_CONTROL 1
#define ENABLE_UART_CONTROL 1
#define ENABLE_WIFI_MANAGEMENT 1

// =============================================================================
// UTILITY MACROS
// =============================================================================

// Array size calculation
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

// String concatenation for build info
#define BUILD_INFO_STRING BUILD_DATE "-" BUILD_TIME

// Pin validation
#define IS_VALID_PIN(pin) ((pin) >= MIN_PIN_NUMBER && (pin) <= MAX_PIN_NUMBER)
#define IS_VALID_DEVICE_ADDRESS(addr)                                          \
  ((addr) >= MIN_DEVICE_ADDRESS && (addr) <= MAX_DEVICE_ADDRESS)
#define IS_VALID_BYTE_COUNT(bytes)                                             \
  ((bytes) >= I2C_MIN_BYTES && (bytes) <= I2C_MAX_BYTES)

// =============================================================================
// LEGACY COMPATIBILITY
// =============================================================================

// For backward compatibility with existing code
#define DEVICE DEVICE_NAME
#define REVISION FIRMWARE_VERSION

#endif // CONFIG_H
