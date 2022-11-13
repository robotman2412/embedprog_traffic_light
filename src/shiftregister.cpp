
#include "shiftregister.hpp"

#define SHIFT_REGISTER_DELAY() delayMicroseconds(1);

void shift_register_send(int dataPin, int serialClockPin, int registerClockPin, size_t length, const bool *data) {
    digitalWrite(serialClockPin,   0);
    digitalWrite(registerClockPin, 0);
    
    for (size_t i = 0; i < length; i++) {
        digitalWrite(dataPin, data[length-i-1]);
        SHIFT_REGISTER_DELAY()
        digitalWrite(serialClockPin, 1);
        SHIFT_REGISTER_DELAY()
        digitalWrite(serialClockPin, 0);
        SHIFT_REGISTER_DELAY()
    }
    
    digitalWrite(registerClockPin, 1);
    SHIFT_REGISTER_DELAY()
    digitalWrite(registerClockPin, 0);
}
