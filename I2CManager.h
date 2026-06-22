#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <ArduinoJson.h>

class I2CManager {
public:
    // Initialise the I2C bus. Call once from setup().
    static void initialize(int sda_pin, int scl_pin, uint32_t frequency = 100000);
    static bool isInitialized();

    // Utility
    static String scan();
    static String readAllRegisters(uint8_t device_address);

    // --- v1.1 transaction handlers ---
    // Each method takes a pre-parsed JsonDocument and writes the JSON response
    // into `out`. Returns true on success.

    // Rule: "dev" is ALWAYS a 0x-prefixed hex string (e.g. "0x0d") in both
    // directions. The firmware rejects integer dev values and always echoes dev
    // as a hex string in responses.

    // i2c_transaction: single write+read, write-only, or read-only bus sequence.
    // doc fields: dev ("0xNN"), write (hex str, optional), read_len (int),
    //             repeated_start (bool, optional), post_write_delay_ms (int, optional)
    //             request_id (number or string, optional) — echoed in the response
    static bool transaction(const JsonDocument& doc, JsonDocument& out);

    // i2c_sequence: ordered list of steps; each step is one doStep (write and/or read).
    // Steps may mix write-only, read-only, and combined pointer+read. rx[] lists only
    // steps with read_len > 0, in order.
    // doc fields: dev ("0xNN"), steps (array of step objects),
    //             request_id (optional) — echoed in the response
    static bool sequence(const JsonDocument& doc, JsonDocument& out);

    // i2c_rmw: read-modify-write on one register.
    // doc fields: dev ("0xNN"), pointer (hex str), mask (hex str), tx (hex str)
    static bool rmw(const JsonDocument& doc, JsonDocument& out);

private:
    static bool _initialized;

    // Parses a hex string "0xABCD..." into a byte array.
    // Returns number of bytes written, or -1 on error.
    static int hexToBytes(const char* hex, uint8_t* buf, int maxLen);

    // Performs one bus sequence: write phase then read phase (or either alone).
    // Serialises read result into rxHex ("0x..." string). Returns error code (0=ok).
    static int doStep(uint8_t devAddr, const uint8_t* writeBytes, int writeLen,
                      int readLen, bool repeatedStart, int postDelayMs,
                      String& rxHex);

    // Error code constants
    static const int ERR_BUF_TOO_LONG = 1;
    static const int ERR_NACK_ADDR = 2;
    static const int ERR_NACK_DATA = 3;
    static const int ERR_BUS_BUSY = 4;
    static const int ERR_SHORT_READ = 5;
    static const int ERR_MIXED_DIRECTION = 6;
};
