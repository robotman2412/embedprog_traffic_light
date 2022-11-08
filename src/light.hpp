
#pragma once

#include <Arduino.h>

// Interval in MILLIS for BLINK TIMER.
#define LIGHT_BLINK_TIME 250

// Yellow is emulated by green and red.
#define LIGHT_YELLOW_EMU -2

// Whether active low is the default.
#define ACTIVE_LOW_DEFAULT true

enum Color {
	Red,
	Yellow,
	Green,
	BlinkYellow,
	Off
};

// A single traffic light.
class Light {
	private:
		int   redPin, yellowPin, greenPin;
		float redPwm, yellowPwm, greenPwm;
		bool  red,    yellow,    green;
		
		// Update PWM for a specific pin.
		void updatePinPwm(bool state, float &pwm);
		// An analogWrite wrapper.
		void write(int pin, float value);
		
		// Last color stateD.
		Color lastColor;
		
	public:
		// The color that this light is in.
		Color color;
		// Whether this light uses active low.
		bool activeLow;
		
		// A light that does nothing.
		Light();
		// A light with only red and green.
		// -1 is not present.
		Light(int redPin, int greenPin);
		// A light with red, yellow and green.
		// -1 is not present.
		Light(int redPin, int yellowPin, int greenPin);
		
		// Update the light's PWM outputs.
		void updatePwm();
		// Called when the blinking timer turns on.
		void blinkOn();
		// Called when the blinking timer turns off.
		void blinkOff();
};

// Starts the task that updates PWM values for lights.
void startPwmTask();

// Register this LIGHT for PWM TASK.
void registerLight(Light *light);

// UnRegister this LIGHT from PWM TASK.
void unregisterLight(Light *light);
