#include "I2CManager.h"
#include "Config.h"

bool I2CManager::_initialized = false;

namespace {

// Echo optional client correlation id into responses (UI pending callbacks).
void copyRequestIdIfPresent(const JsonDocument& doc, JsonDocument& out) {
    if (!doc.containsKey("request_id")) return;
    if (doc["request_id"].isNull()) return;
    if (doc["request_id"].is<const char*>()) {
        out["request_id"] = doc["request_id"].as<const char*>();
    } else {
        out["request_id"] = doc["request_id"].as<int32_t>();
    }
}

}  // namespace

// Format a 7-bit I2C address as "0x0d" for JSON responses.
static void setDevField(JsonDocument& out, uint8_t addr) {
    char buf[7];
    snprintf(buf, sizeof(buf), "0x%02x", addr);
    out["dev"] = buf;
}

// Parse "dev" field: accepts hex string only (e.g. "0x0D").
// Integer or non-hex-string values are rejected.
// Returns 0 and sets ok=false on failure.
static uint8_t parseDevAddr(const JsonVariantConst& v, bool& ok) {
    ok = true;
    const char* s = v.as<const char*>();
    if (!s || s[0] == '\0' || s[0] != '0' || (s[1] != 'x' && s[1] != 'X')) {
        ok = false; return 0;
    }
    uint8_t addr = (uint8_t)strtoul(s, nullptr, 16);
    if (addr < 1 || addr > 127) { ok = false; return 0; }
    return addr;
}


void I2CManager::initialize(int sda_pin, int scl_pin, uint32_t frequency) {
    Wire.begin(sda_pin, scl_pin);
    Wire.setClock(frequency);
    _initialized = true;
}

bool I2CManager::isInitialized() { return _initialized; }

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

int I2CManager::hexToBytes(const char* hex, uint8_t* buf, int maxLen) {
    if (!hex || strncmp(hex, "0x", 2) != 0) return -1;
    const char* p = hex + 2;
    int len = strlen(p);
    if (len == 0 || len % 2 != 0) return -1;
    int n = len / 2;
    if (n > maxLen) return -1;
    for (int i = 0; i < n; i++) {
        char byte_str[3] = { p[i*2], p[i*2+1], '\0' };
        buf[i] = (uint8_t)strtoul(byte_str, nullptr, 16);
    }
    return n;
}

int I2CManager::doStep(uint8_t devAddr, const uint8_t* writeBytes, int writeLen,
                       int readLen, bool repeatedStart, int postDelayMs,
                       String& rxHex) {
#if LABKIT_I2C_TIMING_LOG
    const uint32_t tStep0 = micros();
    uint32_t dtWrite = 0;
    uint32_t dtReadAndHex = 0;
#endif
    // Write phase
    if (writeLen > 0) {
#if LABKIT_I2C_TIMING_LOG
        const uint32_t tw0 = micros();
#endif
        Wire.beginTransmission(devAddr);
        for (int i = 0; i < writeLen; i++) Wire.write(writeBytes[i]);
        uint8_t err = Wire.endTransmission(readLen > 0 && repeatedStart ? false : true);
#if LABKIT_I2C_TIMING_LOG
        dtWrite = micros() - tw0;
#endif
        if (err == 1) return ERR_BUF_TOO_LONG;
        if (err == 2) return ERR_NACK_ADDR;
        if (err == 3) return ERR_NACK_DATA;
        if (err != 0) return ERR_BUS_BUSY;
        if (readLen == 0 && postDelayMs > 0) delay(postDelayMs);
    }

    // Read phase
    if (readLen > 0) {
#if LABKIT_I2C_TIMING_LOG
        const uint32_t tr0 = micros();
#endif
        uint8_t got = Wire.requestFrom(devAddr, (uint8_t)readLen);
        if (got < readLen) {
            // Drain any bytes received before returning error
            while (Wire.available()) Wire.read();
#if LABKIT_I2C_TIMING_LOG
            I2C_TIMING_PRINTF(
                "[I2C-TIMING] doStep write_us=%lu read_err_short got=%u need=%u wl=%d rl=%d\n",
                (unsigned long)dtWrite, (unsigned)got, (unsigned)readLen, writeLen, readLen);
#endif
            return ERR_SHORT_READ;
        }
        // Serialise to "0x" + hex
        rxHex = "0x";
        for (int i = 0; i < readLen; i++) {
            uint8_t b = Wire.read();
            if (b < 0x10) rxHex += "0";
            rxHex += String(b, HEX);
        }
#if LABKIT_I2C_TIMING_LOG
        dtReadAndHex = micros() - tr0;
#endif
    }
#if LABKIT_I2C_TIMING_LOG
    I2C_TIMING_PRINTF(
        "[I2C-TIMING] doStep write_us=%lu read_plus_hex_us=%lu total_us=%lu wl=%d rl=%d rs=%d\n",
        (unsigned long)dtWrite, (unsigned long)dtReadAndHex,
        (unsigned long)(micros() - tStep0), writeLen, readLen, repeatedStart ? 1 : 0);
#endif
    return 0; // ok
}

// ---------------------------------------------------------------------------
// Public: transaction
// ---------------------------------------------------------------------------

bool I2CManager::transaction(const JsonDocument& doc, JsonDocument& out) {
#if LABKIT_I2C_TIMING_LOG
    const uint32_t tTxn0 = micros();
#endif
    copyRequestIdIfPresent(doc, out);
    bool devOk;
    uint8_t devAddr = parseDevAddr(doc["dev"], devOk);
    if (!devOk) {
        out["function"] = "i2c_transaction";
        if (doc["dev"].is<const char*>()) {
            out["dev"] = doc["dev"].as<const char*>();
        }
        out["error"] = "bad dev address";
        return false;
    }

    uint8_t writeBuf[64];
    int writeLen = 0;
    const char* writeHex = doc["write"];
    if (writeHex) {
        writeLen = hexToBytes(writeHex, writeBuf, sizeof(writeBuf));
        if (writeLen < 0) {
            out["function"] = "i2c_transaction";
            setDevField(out, devAddr);
            out["error"] = "bad write hex";
            return false;
        }
    }

    int readLen = doc["read_len"] | 0;
    bool repeatedStart = doc["repeated_start"] | true;
    int postDelay = doc["post_write_delay_ms"] | 0;

#if LABKIT_I2C_TIMING_LOG
    const uint32_t tBeforeDoStep = micros();
#endif
    String rxHex;
    int err = doStep(devAddr, writeLen > 0 ? writeBuf : nullptr, writeLen,
                     readLen, repeatedStart, postDelay, rxHex);
#if LABKIT_I2C_TIMING_LOG
    const uint32_t tAfterDoStep = micros();
    long rid = -1;
    if (doc.containsKey("request_id") && !doc["request_id"].isNull()) {
        if (doc["request_id"].is<int32_t>() || doc["request_id"].is<int>()) {
            rid = doc["request_id"].as<int32_t>();
        }
    }
    I2C_TIMING_PRINTF(
        "[I2C-TIMING] txn parse_setup_us=%lu doStep_us=%lu rid=%ld dev=0x%02x rl=%d wl=%d\n",
        (unsigned long)(tBeforeDoStep - tTxn0), (unsigned long)(tAfterDoStep - tBeforeDoStep),
        rid, (unsigned)devAddr, readLen, writeLen);
#endif

    out["function"] = "i2c_transaction";
    setDevField(out, devAddr);
    if (err != 0) {
        out["error"] = "I2C error";
        out["code"] = err;
        return false;
    }
    if (readLen > 0) {
        out["rx"][0] = rxHex;
    } else {
        out["ok"] = true;
    }
    return true;
}

// ---------------------------------------------------------------------------
// Public: sequence
// ---------------------------------------------------------------------------

bool I2CManager::sequence(const JsonDocument& doc, JsonDocument& out) {
    copyRequestIdIfPresent(doc, out);
    bool devOk;
    uint8_t devAddr = parseDevAddr(doc["dev"], devOk);
    if (!devOk) {
        out["function"] = "i2c_sequence";
        if (doc["dev"].is<const char*>()) {
            out["dev"] = doc["dev"].as<const char*>();
        }
        out["error"] = "bad dev address";
        return false;
    }
    JsonArrayConst steps = doc["steps"].as<JsonArrayConst>();

    out["function"] = "i2c_sequence";
    setDevField(out, devAddr);
    JsonArray rxArr = out.createNestedArray("rx");

    for (JsonObjectConst step : steps) {
        uint8_t writeBuf[64];
        int writeLen = 0;
        const char* writeHex = step["write"];
        if (writeHex) {
            writeLen = hexToBytes(writeHex, writeBuf, sizeof(writeBuf));
            if (writeLen < 0) { out["error"] = "bad write hex in step"; return false; }
        }

        int readLen = step["read_len"] | 0;
        bool repeatedStart = step["repeated_start"] | true;
        int postDelay = step["post_write_delay_ms"] | 0;

        String rxHex;
        int err = doStep(devAddr, writeLen > 0 ? writeBuf : nullptr, writeLen,
                         readLen, repeatedStart, postDelay, rxHex);
        if (err != 0) {
            out["error"] = "I2C error in step";
            out["code"] = err;
            return false;
        }
        if (readLen > 0) rxArr.add(rxHex);
    }

    // Write-only sequences omit rx; any step with read_len>0 appends to rx.
    if (rxArr.size() == 0) {
        out.remove("rx");
        out["ok"] = true;
    }
    return true;
}

// ---------------------------------------------------------------------------
// Public: rmw
// ---------------------------------------------------------------------------

bool I2CManager::rmw(const JsonDocument& doc, JsonDocument& out) {
    copyRequestIdIfPresent(doc, out);
    bool devOk;
    uint8_t devAddr = parseDevAddr(doc["dev"], devOk);
    if (!devOk) { out["error"] = "bad dev address"; return false; }

    uint8_t ptrBuf[2]; int ptrLen;
    uint8_t maskBuf[2]; int maskLen;
    uint8_t txBuf[2];   int txLen;

    ptrLen  = hexToBytes(doc["pointer"], ptrBuf,  sizeof(ptrBuf));
    maskLen = hexToBytes(doc["mask"],    maskBuf, sizeof(maskBuf));
    txLen   = hexToBytes(doc["tx"],      txBuf,   sizeof(txBuf));

    if (ptrLen < 1 || maskLen < 1 || txLen < 1 || ptrLen > 2 || maskLen > 2) {
        out["error"] = "bad rmw params"; return false;
    }

    int regWidth = maskLen; // 1 = 8-bit, 2 = 16-bit

    // Read current value
    Wire.beginTransmission(devAddr);
    for (int i = 0; i < ptrLen; i++) Wire.write(ptrBuf[i]);
    uint8_t err = Wire.endTransmission(false);
    if (err != 0) { out["error"] = "I2C error"; out["code"] = (int)err; return false; }

    uint8_t got = Wire.requestFrom(devAddr, (uint8_t)regWidth);
    if (got < regWidth) { out["error"] = "I2C error"; out["code"] = ERR_SHORT_READ; return false; }

    uint8_t curBuf[2] = {0, 0};
    for (int i = 0; i < regWidth; i++) curBuf[i] = Wire.read();

    // Merge: (current & ~mask) | (tx & mask)
    uint8_t newBuf[2] = {0, 0};
    for (int i = 0; i < regWidth; i++) {
        uint8_t m = (i < maskLen) ? maskBuf[i] : 0;
        uint8_t t = (i < txLen)   ? txBuf[i]   : 0;
        newBuf[i] = (curBuf[i] & ~m) | (t & m);
    }

    // Write back
    Wire.beginTransmission(devAddr);
    for (int i = 0; i < ptrLen; i++) Wire.write(ptrBuf[i]);
    for (int i = 0; i < regWidth; i++) Wire.write(newBuf[i]);
    err = Wire.endTransmission(true);
    if (err != 0) { out["error"] = "I2C error on write"; out["code"] = (int)err; return false; }

    out["function"] = "i2c_rmw";
    setDevField(out, devAddr);
    out["ok"] = true;
    return true;
}

// ---------------------------------------------------------------------------
// Utility methods (unchanged behaviour)
// ---------------------------------------------------------------------------

String I2CManager::scan() {
    String found = "";
    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            if (found.length() > 0) found += ",";
            found += "0x";
            if (addr < 0x10) found += "0";
            found += String(addr, HEX);
        }
    }
    return found.length() ? found : "none";
}

String I2CManager::readAllRegisters(uint8_t device_address) {
    String result = "{";
    for (int reg = 0; reg < 256; reg++) {
        Wire.beginTransmission(device_address);
        Wire.write(reg);
        if (Wire.endTransmission(false) == 0) {
            if (Wire.requestFrom(device_address, (uint8_t)1) == 1) {
                uint8_t val = Wire.read();
                if (result.length() > 1) result += ",";
                result += "\"0x";
                if (reg < 0x10) result += "0";
                result += String(reg, HEX) + "\":";
                result += String(val);
            }
        }
    }
    result += "}";
    return result;
}
