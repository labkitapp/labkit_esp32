#ifndef SPI_MANAGER_H
#define SPI_MANAGER_H

#include "Config.h"
#include "ErrorHandler.h"
#include <SPI.h>
#include <Arduino.h>

/**
 * @brief SPI Manager class for handling all SPI operations
 * 
 * This class provides a centralized interface for SPI communication,
 * including read/write operations and device management.
 * It encapsulates all SPI-related functionality in a single module.
 */
class SPIManager {
public:
    /**
     * @brief Initialize the SPI manager
     * @param ss_pin Slave Select pin number
     * @param frequency SPI frequency in Hz (default: 1000000)
     * @param mode SPI mode (default: SPI_MODE0)
     */
    static void initialize(int ss_pin, uint32_t frequency = SPI_FREQUENCY_HZ, uint8_t mode = SPI_MODE0);
    
    /**
     * @brief Execute SPI command
     * @param data Array of data to send
     * @param len Number of bytes to send
     * @param received_data Array to store received data (optional)
     * @return ErrorCode indicating success or failure
     */
    static ErrorCode command(uint8_t *data, uint8_t len, uint8_t *received_data = nullptr);
    
    /**
     * @brief Read data from SPI device
     * @param data Array to store received data
     * @param len Number of bytes to read
     * @return ErrorCode indicating success or failure
     */
    static ErrorCode read(uint8_t *data, uint8_t len);
    
    /**
     * @brief Write data to SPI device
     * @param data Array of data to send
     * @param len Number of bytes to write
     * @return ErrorCode indicating success or failure
     */
    static ErrorCode write(uint8_t *data, uint8_t len);
    
    /**
     * @brief Transfer data (write and read simultaneously)
     * @param tx_data Array of data to send
     * @param rx_data Array to store received data
     * @param len Number of bytes to transfer
     * @return ErrorCode indicating success or failure
     */
    static ErrorCode transfer(uint8_t *tx_data, uint8_t *rx_data, uint8_t len);
    
    /**
     * @brief Check if SPI is initialized
     * @return true if initialized, false otherwise
     */
    static bool isInitialized();
    
    /**
     * @brief Get the last received data
     * @return Pointer to received data buffer
     */
    static uint8_t* getLastReceivedData();
    
    /**
     * @brief Get the size of the received data buffer
     * @return Size of the buffer
     */
    static uint8_t getReceivedDataSize();

private:
    static bool _initialized;
    static SPIClass* _spi;
    static int _ss_pin;
    static uint8_t _received_data[SPI_RECEIVE_BUFFER_SIZE];
    static uint8_t _received_size;
    
    /**
     * @brief Validate SPI parameters
     * @param data Data array
     * @param len Number of bytes
     * @return ErrorCode indicating validation result
     */
    static ErrorCode validateParameters(uint8_t *data, uint8_t len);
    
    /**
     * @brief Begin SPI transaction
     * @return ErrorCode indicating success or failure
     */
    static ErrorCode beginTransaction();
    
    /**
     * @brief End SPI transaction
     */
    static void endTransaction();
};

// Legacy function wrappers for backward compatibility
void spiCommand(uint8_t *data, uint8_t len);

#endif // SPI_MANAGER_H
