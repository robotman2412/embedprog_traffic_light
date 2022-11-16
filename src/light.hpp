
#pragma once

#include <Arduino.h>

// Interval in MILLIS for BLINK TIMER.
#define LIGHT_BLINK_TIME 250

enum Color {
	Red,
	Yellow,
	Green,
	Off
};

// A single traffic light.
class Light {
	private:
		bool *redPtr, *yellowPtr, *greenPtr;
		bool  red,     yellow,     green;
		
	public:
		// The color that this light is in.
		Color color;
		
		// A light that does nothing.
		Light();
		// A light with only red and green.
		// -1 is not present.
		Light(bool *redPin, bool *greenPin);
		// A light with red, yellow and green.
		// -1 is not present.
		Light(bool *redPin, bool *yellowPin, bool *greenPin);
		
		// Update the light's pin outputs.
		void update(bool blinkStatus);
};

// Update all the lights.
void updateLights();

// Register this LIGHT for updating in updateLights().
Light *registerLight(Light *light);
