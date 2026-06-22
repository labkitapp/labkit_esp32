#ifndef PIN_MANAGER_H
#define PIN_MANAGER_H

#include "Config.h"
#include "ErrorHandler.h"
#include <Arduino.h>

/**
 * @brief Pin Manager class for handling all GPIO operations
 * 
 * This class provides a centralized interface for GPIO operations,
 * including pin configuration, reading, writing, and state monitoring.
 * It encapsulates all pin-related functionality in a single module.
 */
class PinManager {
public:
    // Pin modes
    enum class PinMode {
        PIN_OUTPUT = 0,
        PIN_INPUT = 1,
        PIN_HIGH_Z = 2
    };
    
    /**
     * @brief Initialize the pin manager
     */
    static void initialize();
    
    /**
     * @brief Set pin mode
     * @param pin Pin number
     * @param mode Pin mode (OUTPUT, INPUT, HIGH_Z)
     * @return ErrorCode indicating success or failure
     */
    static ErrorCode setPinMode(int pin, PinMode mode);
    
    /**
     * @brief Write digital value to pin
     * @param pin Pin number
     * @param value Digital value (0 or 1)
     * @return ErrorCode indicating success or failure
     */
    static ErrorCode writeDigital(int pin, int value);
    
    /**
     * @brief Read digital value from pin
     * @param pin Pin number
     * @param value Pointer to store read value
     * @return ErrorCode indicating success or failure
     */
    static ErrorCode readDigital(int pin, int* value);
    
    /**
     * @brief Read analog value from pin
     * @param pin Pin number
     * @param value Pointer to store read value
     * @return ErrorCode indicating success or failure
     */
    static ErrorCode readAnalog(int pin, int* value);
    
    /**
     * @brief Get pin mode
     * @param pin Pin number
     * @return PinMode of the pin
     */
    static PinMode getPinMode(int pin);
    
    /**
     * @brief Get pin value
     * @param pin Pin number
     * @return Current pin value
     */
    static int getPinValue(int pin);
    
    /**
     * @brief Check if pin is configured as input
     * @param pin Pin number
     * @return true if input, false otherwise
     */
    static bool isInputPin(int pin);
    
    /**
     * @brief Check if pin is configured as output
     * @param pin Pin number
     * @return true if output, false otherwise
     */
    static bool isOutputPin(int pin);
    
    /**
     * @brief Check if pin is in high impedance mode
     * @param pin Pin number
     * @return true if high-z, false otherwise
     */
    static bool isHighZPin(int pin);
    
    /**
     * @brief Initialize all pins to high impedance
     */
    static void initializeAllPinsToHighZ();
    
    /**
     * @brief Check for pin state changes
     * @param changedPins Array to store changed pin numbers
     * @param changedValues Array to store changed pin values
     * @return Number of pins that changed
     */
    static int checkPinStates(int* changedPins, int* changedValues);
    
    /**
     * @brief Get pin mode for a specific pin
     * @param pin Pin number
     * @return Pin mode (0=output, 1=input, 2=highz)
     */
    static int getPinModeValue(int pin);
    
    /**
     * @brief Set analog pin for reading
     * @param pin Analog pin number
     * @return ErrorCode indicating success or failure
     */
    static ErrorCode setAnalogPin(int pin);
    
    /**
     * @brief Get current analog pin
     * @return Current analog pin number
     */
    static int getAnalogPin();

private:
    static int _input_pins[MAX_PIN_ID][2]; // [pin][0] = mode, [pin][1] = value
    static int _analog_pin;
    static unsigned long _last_pin_check_time;
    
    /**
     * @brief Validate pin number
     * @param pin Pin number
     * @return ErrorCode indicating validation result
     */
    static ErrorCode validatePin(int pin);
    
    /**
     * @brief Update stored pin value
     * @param pin Pin number
     * @param value New value
     */
    static void updatePinValue(int pin, int value);
    
};


#endif // PIN_MANAGER_H
