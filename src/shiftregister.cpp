
#include "shiftregister.hpp"
#include <Arduino.h>

// A shift register is pretty fast.
// This delay is one microsecond (0.000001 second)
#define SHIFT_REGISTER_DELAY() delayMicroseconds(1);

void shift_register_send(int dataPin, int serialClockPin, int registerClockPin, int length, const bool *data) {
    // The shift register is a simple memory device.
    // The serial clock causes the shift register to accept one bit.
    // The register clock causes the shift register to present all the bits.
    // Clocks are counted when they change from LOW to HIGH.
    
    // Set the clocks to low, just in case they weren't.
    digitalWrite(serialClockPin,   LOW);
    digitalWrite(registerClockPin, LOW);
    
    // This FOR loop is in reverse, so that the first bit in data is the first bit physically.
    for (int i = length - 1; i >= 0; i--) {
        // Write the data bit to the data input line of the shift register.
        digitalWrite(dataPin, data[i]);
        SHIFT_REGISTER_DELAY()
        
        // Trigger the serial clock to make the shift register read the new data.
        digitalWrite(serialClockPin, HIGH);
        SHIFT_REGISTER_DELAY()
        digitalWrite(serialClockPin, LOW);
        SHIFT_REGISTER_DELAY()
    }
    
    // Activate the register clock to present the new data.
    digitalWrite(registerClockPin, HIGH);
    SHIFT_REGISTER_DELAY()
    digitalWrite(registerClockPin, LOW);
}
