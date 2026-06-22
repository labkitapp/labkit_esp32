#include "SPIManager.h"
#include <SPI.h>

// Static member initialization
bool SPIManager::_initialized = false;
SPIClass* SPIManager::_spi = nullptr;
int SPIManager::_ss_pin = -1;
uint8_t SPIManager::_received_data[SPI_RECEIVE_BUFFER_SIZE] = {0};
uint8_t SPIManager::_received_size = 0;

void SPIManager::initialize(int ss_pin, uint32_t frequency, uint8_t mode) {
    if (_spi == nullptr) {
        _spi = new SPIClass(VSPI);
    }
    
    _ss_pin = ss_pin;
    _spi->begin(SCK, MISO, MOSI, _ss_pin);
    _spi->setDataMode(mode);
    _spi->setFrequency(frequency);
    pinMode(_ss_pin, OUTPUT);
    digitalWrite(_ss_pin, HIGH); // SS high by default
    
    _initialized = true;
    
    #if DEBUG_ENABLED
    DEBUG_PRINTF("SPI Manager initialized: SS=%d, Freq=%d Hz, Mode=%d\n", 
                 ss_pin, frequency, mode);
    #endif
}

ErrorCode SPIManager::command(uint8_t *data, uint8_t len, uint8_t *received_data) {
    if (!_initialized) {
        return ErrorCode::SPI_NOT_INITIALIZED;
    }
    
    ErrorCode validation = validateParameters(data, len);
    if (validation != ErrorCode::SUCCESS) {
        return validation;
    }
    
    ErrorCode result = beginTransaction();
    if (result != ErrorCode::SUCCESS) {
        return result;
    }
    
    // Transfer data
    for (uint8_t i = 0; i < len; i++) {
        _received_data[i] = _spi->transfer(data[i]);
    }
    _received_size = len;
    
    // Copy received data if requested
    if (received_data != nullptr) {
        memcpy(received_data, _received_data, len);
    }
    
    endTransaction();
    
    #if DEBUG_ENABLED
    DEBUG_PRINTF("SPI Command: Sent %d bytes, received %d bytes\n", len, _received_size);
    #endif
    
    return ErrorCode::SUCCESS;
}

ErrorCode SPIManager::write(uint8_t *data, uint8_t len) {
    if (!_initialized) {
        return ErrorCode::SPI_NOT_INITIALIZED;
    }
    
    ErrorCode validation = validateParameters(data, len);
    if (validation != ErrorCode::SUCCESS) {
        return validation;
    }
    
    ErrorCode result = beginTransaction();
    if (result != ErrorCode::SUCCESS) {
        return result;
    }
    
    // Write data (ignore received data)
    for (uint8_t i = 0; i < len; i++) {
        _spi->transfer(data[i]);
    }
    _received_size = 0;
    
    endTransaction();
    
    #if DEBUG_ENABLED
    DEBUG_PRINTF("SPI Write: Sent %d bytes\n", len);
    #endif
    
    return ErrorCode::SUCCESS;
}

ErrorCode SPIManager::transfer(uint8_t *tx_data, uint8_t *rx_data, uint8_t len) {
    if (!_initialized) {
        return ErrorCode::SPI_NOT_INITIALIZED;
    }
    
    ErrorCode validation = validateParameters(tx_data, len);
    if (validation != ErrorCode::SUCCESS) {
        return validation;
    }
    
    if (rx_data == nullptr) {
        return ErrorCode::INVALID_PARAMETER;
    }
    
    ErrorCode result = beginTransaction();
    if (result != ErrorCode::SUCCESS) {
        return result;
    }
    
    // Transfer data
    for (uint8_t i = 0; i < len; i++) {
        rx_data[i] = _spi->transfer(tx_data[i]);
    }
    _received_size = len;
    
    endTransaction();
    
    #if DEBUG_ENABLED
    DEBUG_PRINTF("SPI Transfer: Sent %d bytes, received %d bytes\n", len, len);
    #endif
    
    return ErrorCode::SUCCESS;
}

bool SPIManager::isInitialized() {
    return _initialized;
}

uint8_t* SPIManager::getLastReceivedData() {
    return _received_data;
}

uint8_t SPIManager::getReceivedDataSize() {
    return _received_size;
}

ErrorCode SPIManager::validateParameters(uint8_t *data, uint8_t len) {
    if (data == nullptr) {
        return ErrorCode::INVALID_PARAMETER;
    }
    
    if (len == 0 || len > SPI_MAX_BYTES) {
        return ErrorCode::SPI_INVALID_BYTE_COUNT;
    }
    
    return ErrorCode::SUCCESS;
}

ErrorCode SPIManager::beginTransaction() {
    if (_spi == nullptr || _ss_pin == -1) {
        return ErrorCode::SPI_NOT_INITIALIZED;
    }
    
    digitalWrite(_ss_pin, LOW); // Pull SS low to start transaction
    _spi->beginTransaction(SPISettings(SPI_FREQUENCY_HZ, MSBFIRST, SPI_MODE0));
    
    return ErrorCode::SUCCESS;
}

void SPIManager::endTransaction() {
    if (_spi != nullptr && _ss_pin != -1) {
        _spi->endTransaction();
        digitalWrite(_ss_pin, HIGH); // Pull SS high to end transaction
    }
}

// Legacy function wrapper for backward compatibility
void spiCommand(uint8_t *data, uint8_t len) {
    SPIManager::command(data, len);
}
