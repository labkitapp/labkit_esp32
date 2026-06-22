#include "PinManager.h"

// Static member initialization
int PinManager::_input_pins[MAX_PIN_ID][2] = {0};
int PinManager::_analog_pin = A0;
unsigned long PinManager::_last_pin_check_time = 0;

void PinManager::initialize() {
    initializeAllPinsToHighZ();
    _last_pin_check_time = millis();
    
    #if DEBUG_ENABLED
    DEBUG_PRINTF("Pin Manager initialized with %d pins\n", MAX_PIN_ID);
    #endif
}

ErrorCode PinManager::setPinMode(int pin, PinMode mode) {
    ErrorCode validation = validatePin(pin);
    if (validation != ErrorCode::SUCCESS) {
        return validation;
    }
    
    switch (mode) {
        case PinMode::PIN_OUTPUT:
            pinMode(pin, OUTPUT);
            _input_pins[pin][0] = 0;
            _input_pins[pin][1] = 0;
            break;
            
        case PinMode::PIN_INPUT:
            pinMode(pin, INPUT);
            _input_pins[pin][0] = 1;
            _input_pins[pin][1] = digitalRead(pin);
            break;
            
        case PinMode::PIN_HIGH_Z:
            pinMode(pin, INPUT); // Set as input but don't enable pull-up/down
            _input_pins[pin][0] = 2;
            _input_pins[pin][1] = digitalRead(pin);
            break;
            
        default:
            return ErrorCode::INVALID_PARAMETER;
    }
    
    #if DEBUG_ENABLED
    const char* modeStr = (mode == PinMode::PIN_OUTPUT) ? "OUTPUT" : 
                         (mode == PinMode::PIN_INPUT) ? "INPUT" : "HIGH_Z";
    DEBUG_PRINTF("Pin %d set to %s mode\n", pin, modeStr);
    #endif
    
    return ErrorCode::SUCCESS;
}

ErrorCode PinManager::writeDigital(int pin, int value) {
    ErrorCode validation = validatePin(pin);
    if (validation != ErrorCode::SUCCESS) {
        return validation;
    }
    
    if (_input_pins[pin][0] != 0) {
        return ErrorCode::PIN_NOT_CONFIGURED_AS_OUTPUT;
    }
    
    digitalWrite(pin, value);
    updatePinValue(pin, value);
    
    #if DEBUG_ENABLED
    DEBUG_PRINTF("Pin %d written: %d\n", pin, value);
    #endif
    
    return ErrorCode::SUCCESS;
}

ErrorCode PinManager::readDigital(int pin, int* value) {
    ErrorCode validation = validatePin(pin);
    if (validation != ErrorCode::SUCCESS) {
        return validation;
    }
    
    if (value == nullptr) {
        return ErrorCode::INVALID_PARAMETER;
    }
    
    if (_input_pins[pin][0] != 1) {
        return ErrorCode::PIN_NOT_CONFIGURED_AS_INPUT;
    }
    
    *value = digitalRead(pin);
    updatePinValue(pin, *value);
    
    #if DEBUG_ENABLED
    DEBUG_PRINTF("Pin %d read: %d\n", pin, *value);
    #endif
    
    return ErrorCode::SUCCESS;
}

ErrorCode PinManager::readAnalog(int pin, int* value) {
    
    if (value == nullptr) {
        return ErrorCode::INVALID_PARAMETER;
    }
    
    *value = analogRead(pin);
    
    #if DEBUG_ENABLED
    DEBUG_PRINTF("Analog pin %d read: %d\n", pin, *value);
    #endif
    
    return ErrorCode::SUCCESS;
}

PinManager::PinMode PinManager::getPinMode(int pin) {
    if (pin < 0 || pin >= MAX_PIN_ID) {
        return PinMode::PIN_HIGH_Z;
    }
    
    switch (_input_pins[pin][0]) {
        case 0: return PinMode::PIN_OUTPUT;
        case 1: return PinMode::PIN_INPUT;
        case 2: return PinMode::PIN_HIGH_Z;
        default: return PinMode::PIN_HIGH_Z;
    }
}

int PinManager::getPinValue(int pin) {
    if (pin < 0 || pin >= MAX_PIN_ID) {
        return 0;
    }
    return _input_pins[pin][1];
}

bool PinManager::isInputPin(int pin) {
    return getPinMode(pin) == PinMode::PIN_INPUT;
}

bool PinManager::isOutputPin(int pin) {
    return getPinMode(pin) == PinMode::PIN_OUTPUT;
}

bool PinManager::isHighZPin(int pin) {
    return getPinMode(pin) == PinMode::PIN_HIGH_Z;
}

void PinManager::initializeAllPinsToHighZ() {
    for (size_t i = 0; i < MAX_PIN_ID; i++) {
        _input_pins[i][0] = 2;  // Set all pins to high impedance by default
        _input_pins[i][1] = 0;  // Initialize pin value to 0
    }
    
    #if DEBUG_ENABLED
    DEBUG_PRINTF("All %d pins initialized to HIGH_Z mode\n", MAX_PIN_ID);
    #endif
}

int PinManager::checkPinStates(int* changedPins, int* changedValues) {
    if (changedPins == nullptr || changedValues == nullptr) {
        return 0;
    }
    
    int changeCount = 0;
    
    for (size_t i = 0; i < MAX_PIN_ID; i++) {
        if (_input_pins[i][0] == 1) {  // Only monitor pins configured as inputs
            int val = digitalRead(i);
            if (_input_pins[i][1] != val) {
                _input_pins[i][1] = val;
                if (changeCount < MAX_PIN_ID) {  // Prevent buffer overflow
                    changedPins[changeCount] = i;
                    changedValues[changeCount] = val;
                    changeCount++;
                }
            }
        }
    }
    
    return changeCount;
}

int PinManager::getPinModeValue(int pin) {
    if (pin < 0 || pin >= MAX_PIN_ID) {
        return 2; // HIGH_Z
    }
    return _input_pins[pin][0];
}

ErrorCode PinManager::setAnalogPin(int pin) {
    
    _analog_pin = pin;
    
    #if DEBUG_ENABLED
    DEBUG_PRINTF("Analog pin set to: %d\n", pin);
    #endif
    
    return ErrorCode::SUCCESS;
}

int PinManager::getAnalogPin() {
    return _analog_pin;
}

ErrorCode PinManager::validatePin(int pin) {
    if (pin < 0 || pin >= MAX_PIN_ID) {
        return ErrorCode::INVALID_PIN_NUMBER;
    }
    return ErrorCode::SUCCESS;
}

void PinManager::updatePinValue(int pin, int value) {
    if (pin >= 0 && pin < MAX_PIN_ID) {
        _input_pins[pin][1] = value;
    }
}

