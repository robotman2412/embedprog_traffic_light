
#include "light.hpp"
#include <vector>



// A list of all lights in the traffic system.
static std::vector<Light *> lightsList;



// A light that does nothing.
Light::Light() {
	this->redPtr    = NULL;
	this->yellowPtr = NULL;
	this->greenPtr  = NULL;
	red       = false;
	yellow    = false;
	green     = false;
	color     = Color::Off;
}

// A light with only red and green.
Light::Light(bool *redPtr, bool *greenPtr) {
	this->redPtr    = redPtr;
	this->yellowPtr = NULL;
	this->greenPtr  = greenPtr;
	red       = false;
	yellow    = false;
	green     = false;
	color     = Color::Red;
}

// A light with red, yellow and green.
Light::Light(bool *redPtr, bool *yellowPtr, bool *greenPtr) {
	this->redPtr    = redPtr;
	this->yellowPtr = yellowPtr;
	this->greenPtr  = greenPtr;
	red       = false;
	yellow    = false;
	green     = false;
	color     = Color::Red;
}

// Update the light's LED outputs based on it's target color.
void Light::update(bool blinkStatus) {
	// Calculate LED values for current color.
	switch (color) {
		case Color::Off:
			red    = false;
			yellow = false;
			green  = false;
			break;
			
		case Color::Red:
			red    = true;
			yellow = false;
			green  = false;
			break;
			
		case Color::Yellow:
			red    = false;
			yellow = true;
			// Is there a yellow pin?
			if (!yellowPtr) {
				// If not, green blinks when yellow.
				green = blinkStatus;
			} else {
				// If so, green behaves normally.
				green  = false;
			}
			break;
			
		case Color::Green:
			red    = false;
			yellow = false;
			green  = true;
			break;
			
		default: break;
	}
	
	// Update the red pin.
	if (redPtr) *redPtr = red;
	
	// Update the yellow pin.
	if (yellowPtr) *yellowPtr = yellow;
	
	// Update the green pin.
	if (greenPtr) *greenPtr = green;
}



// Update all the lights.
void updateLights() {
	// The last time at which the lights blinked.
	static uint64_t lastBlinkTimer = 0;
	// Whether blinking lights are currently ON.
	static bool blinkingStatus = 0;
	
	// Has LIGHT_BLINK_TIME passed yet?
	if (millis() > lastBlinkTimer + LIGHT_BLINK_TIME) {
		// If so, reset the timer.
		lastBlinkTimer = millis();
		// And blink the lights.
		blinkingStatus = !blinkingStatus;
	}
	
	// Iterate over the list of lights.
	for (auto iterator = lightsList.begin(); iterator != lightsList.end(); iterator ++) {
		// Update each one.
		(*iterator)->update(blinkingStatus);
	}
}




// Register a light to the updater task.
Light *registerLight(Light *light) {
	lightsList.push_back(light);
	return light;
}
