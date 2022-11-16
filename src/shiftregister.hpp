
#pragma once

#include <Arduino.h>

void shift_register_send(int dataPin, int serialClockPin, int registerClockPin, int length, const bool *data);
