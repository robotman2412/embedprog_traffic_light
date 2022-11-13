
#pragma once

#include <Arduino.h>

void shift_register_send(int dataPin, int serialClockPin, int registerClockPin, size_t length, const bool *data);
