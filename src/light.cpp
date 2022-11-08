
#include "light.hpp"
#include <math.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <vector>
#include <algorithm>



#define MTX_WAIT_TIME pdMS_TO_TICKS(50)
static const char *TAG = "light";

static std::vector<Light *> lightPwmList;
static SemaphoreHandle_t pwmMutex;



#define PWM_COEFF 0.2
#define PWM_RES   255

// Update PWM for a specific pin.
void Light::updatePinPwm(bool state, float &pwm) {
	// pwm = state;
	if (state) {
		pwm  = pwm + (1 - pwm) * PWM_COEFF;
	} else {
		pwm *= (1 - PWM_COEFF);
	}
}

// An analogWrite wrapper.
void Light::write(int pin, float value) {
	if (activeLow) value = 1 - value;
	analogWrite(pin, PWM_RES * value);
}



// A light that does nothing.
Light::Light() {
	redPin    = -1;
	yellowPin = -1;
	greenPin  = -1;
	red       = false;
	yellow    = false;
	green     = false;
	redPwm    = 0;
	yellowPwm = 0;
	greenPwm  = 0;
	color     = Color::Off;
	lastColor = color;
	activeLow = ACTIVE_LOW_DEFAULT;
}

// A light with only red and green.
// -1 is not present.
Light::Light(int redPin, int greenPin) {
	this->redPin    = redPin;
	this->yellowPin = -1;
	this->greenPin  = greenPin;
	red       = false;
	yellow    = false;
	green     = false;
	redPwm    = 0;
	yellowPwm = 0;
	greenPwm  = 0;
	color     = Color::Off;
	lastColor = color;
	activeLow = ACTIVE_LOW_DEFAULT;
}

// A light with red, yellow and green.
// -1 is not present.
Light::Light(int redPin, int yellowPin, int greenPin) {
	this->redPin    = redPin;
	this->yellowPin = yellowPin;
	this->greenPin  = greenPin;
	red       = false;
	yellow    = false;
	green     = false;
	redPwm    = 0;
	yellowPwm = 0;
	greenPwm  = 0;
	color     = Color::Off;
	lastColor = color;
	activeLow = ACTIVE_LOW_DEFAULT;
}

// Update the light's PWM outputs.
void Light::updatePwm() {
	if (color != lastColor) {
		switch (color) {
			case Color::Off:
				red    = true;
				yellow = true;
				green  = true;
				break;
				
			case Color::Red:
				red    = true;
				yellow = false;
				green  = false;
				break;
				
			case Color::Yellow:
				red    = false;
				yellow = true;
				green  = false;
				break;
				
			case Color::Green:
				red    = false;
				yellow = false;
				green  = true;
				break;
				
			default: break;
		}
		lastColor = color;
	}
	
	updatePinPwm(red,    redPwm);
	updatePinPwm(yellow, yellowPwm);
	updatePinPwm(green,  greenPwm);
	
	if (redPin >= 0 && greenPin >= 0 && yellowPin == LIGHT_YELLOW_EMU) {
		float redThing   = redPwm   + yellowPwm * 0.6;
		float greenThing = greenPwm + yellowPwm * 1.0;
		// float redThing   = 0.0;
		// float greenThing = 0.1;
		// redThing   *= 0.05;
		// greenThing *= 0.05;
		write(redPin,   redThing);
		write(greenPin, greenThing);
	} else {
		if (redPin    >= 0)
			write(redPin,    redPwm);
		if (yellowPin >= 0)
			write(yellowPin, yellowPwm);
		if (greenPin  >= 0)
			write(greenPin,  greenPwm);
	}
}

// Called when the blinking timer turns on.
void Light::blinkOn() {
	if (color == Color::Yellow && yellowPin == -1) {
		green = true;
	}
}

// Called when the blinking timer turns off.
void Light::blinkOff() {
	if (color == Color::Yellow && yellowPin == -1) {
		green = false;
	}
}



// A handle with pwmTask in it.
TaskHandle_t pwmTaskHandle;

// The task code that automatically updates the PWM bits.
static void pwmTask(void *arg) {
	uint64_t lastBlinkTimer = millis();
	bool     lastBlink      = false;
	
	while (true) {
		// Obtain mutex.
		if (!xSemaphoreTake(pwmMutex, MTX_WAIT_TIME)) {
			ESP_LOGE(TAG, "Mutex aquire timeout!");
			abort();
		}
		
		if (millis() > lastBlinkTimer + LIGHT_BLINK_TIME) {
			lastBlinkTimer  = millis();
			lastBlink      ^= 1;
			
			if (lastBlink) {
				// Blink turned ON.
				for (auto iter = lightPwmList.begin(); iter != lightPwmList.end(); iter ++) {
					(*iter)->blinkOn();
				}
			} else {
				// Blink turned OFF.
				for (auto iter = lightPwmList.begin(); iter != lightPwmList.end(); iter ++) {
					(*iter)->blinkOff();
				}
			}
		}
		
		// Update lights.
		for (auto iter = lightPwmList.begin(); iter != lightPwmList.end(); iter ++) {
			(*iter)->updatePwm();
		}
		
		// Release mutex.
		xSemaphoreGive(pwmMutex);
		
		// Wait for 10 ms.
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}

// Starts the task that updates PWM values for lights.
void startPwmTask() {
	// Make sure mutex is initialised.
	pwmMutex = xSemaphoreCreateMutex();
	
	// Register a task.
	if (xTaskCreate(pwmTask, "PWM task", 4096, NULL, 1, &pwmTaskHandle) != pdPASS) {
		ESP_LOGE(TAG, "Task creation error!");
		abort();
	}
}



// Register this LIGHT for PWM TASK.
void registerLight(Light *light) {
	// Obtain mutex.
	if (!xSemaphoreTake(pwmMutex, MTX_WAIT_TIME)) {
		ESP_LOGE(TAG, "Mutex aquire timeout!");
		abort();
	}
	
	// Add to the list.
	lightPwmList.push_back(light);
	
	// Release mutex.
	xSemaphoreGive(pwmMutex);
}

// UnRegister this LIGHT from PWM TASK.
void unregisterLight(Light *light) {
	// Obtain mutex.
	if (!xSemaphoreTake(pwmMutex, MTX_WAIT_TIME)) {
		ESP_LOGE(TAG, "Mutex aquire timeout!");
		abort();
	}
	
	// Delete from list.
	lightPwmList.erase(std::remove(lightPwmList.begin(), lightPwmList.end(), light), lightPwmList.end());
	
	// Release mutex.
	xSemaphoreGive(pwmMutex);
}
