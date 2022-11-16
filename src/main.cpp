
#include "main.hpp"

// The amount of bits to use for the shift register.
#define SR_BITS 40
// The serial clock pin for the shift register.
#define SR_SCK  18
// The register clock pin for the shift register.
#define SR_RCK  23
// The data input pin for the shift register.
#define SR_DAT  19

// The bits to send to the shift register later on.
bool srBits[SR_BITS];

// There are 8 pedestrian LEDs, but one phase.
// This helper is used to extend one phase to 8 LEDs.
bool pedestianRed;
// There are 8 pedestrian LEDs, but one phase.
// This helper is used to extend one phase to 8 LEDs.
bool pedestianGreen;

Phase phases[9];

const int buttonPins[9] = {
	22,
	0,
	
	25,
	27,
	
	12,
	2,
	
	16,
	4,
	
	17
};

System trafficsys;

// Bitmap: Straight and right #1
#define MASK_SR1 (1 << 0)
// Bitmap: Left #1
#define MASK_LT1 (1 << 1)
// Bitmap: Straight and right #2
#define MASK_SR2 (1 << 2)
// Bitmap: Left #2
#define MASK_LT2 (1 << 3)
// Bitmap: Straight and right #3
#define MASK_SR3 (1 << 4)
// Bitmap: Left #3
#define MASK_LT3 (1 << 5)
// Bitmap: Straight and right #4
#define MASK_SR4 (1 << 6)
// Bitmap: Left #4
#define MASK_LT4 (1 << 7)
// Bitmap: Pedestrians
#define MASK_PED (1 << 8)

// Bitmap: All phases.
#define MASK_ALL 0x1ff

void setup() {
	
	// LEDs per traffic light are ordered Red, Yellow, Green.
	// Traffic lights are ordered Straight/Right followed by Left, per direction.
	// Directions 1 and 3 oppose each other.
	
	// Create phases.
	phases[0] = Phase("Straight/Right #1", registerLight(new Light(srBits +  0, srBits +  1, srBits +  2)));
	phases[0].exclusive = MASK_ALL & ~MASK_SR1 & ~MASK_SR3 & ~MASK_LT1;
	phases[1] = Phase("Left turn #1",      registerLight(new Light(srBits +  3, srBits +  4, srBits +  5)));
	phases[1].exclusive = MASK_ALL & ~MASK_LT1 & ~MASK_LT3 & ~MASK_SR1;
	
	phases[2] = Phase("Straight/Right #2", registerLight(new Light(srBits +  6, srBits +  7, srBits +  8)));
	phases[2].exclusive = MASK_ALL & ~MASK_SR2 & ~MASK_SR4 & ~MASK_LT2;
	phases[3] = Phase("Left turn #2",      registerLight(new Light(srBits +  9, srBits + 10, srBits + 11)));
	phases[3].exclusive = MASK_ALL & ~MASK_LT2 & ~MASK_LT4 & ~MASK_SR2;
	
	phases[4] = Phase("Straight/Right #3", registerLight(new Light(srBits + 12, srBits + 13, srBits + 14)));
	phases[4].exclusive = MASK_ALL & ~MASK_SR1 & ~MASK_SR3 & ~MASK_LT3;
	phases[5] = Phase("Left turn #3",      registerLight(new Light(srBits + 15, srBits + 16, srBits + 17)));
	phases[5].exclusive = MASK_ALL & ~MASK_LT1 & ~MASK_LT3 & ~MASK_SR3;
	
	phases[6] = Phase("Straight/Right #4", registerLight(new Light(srBits + 18, srBits + 19, srBits + 20)));
	phases[6].exclusive = MASK_ALL & ~MASK_SR2 & ~MASK_SR4 & ~MASK_LT4;
	phases[7] = Phase("Left turn #4",      registerLight(new Light(srBits + 21, srBits + 22, srBits + 23)));
	phases[7].exclusive = MASK_ALL & ~MASK_LT2 & ~MASK_LT4 & ~MASK_SR4;
	
	phases[8] = Phase("Pedestrian", registerLight(new Light(&pedestianRed, &pedestianGreen)));
	phases[8].exclusive = MASK_ALL & ~MASK_PED;
	phases[8].priority = 2;
	
	// Traffic reed switches are active low.
	for (int i = 0; i < 8; i++) {
		pinMode(buttonPins[i], INPUT_PULLUP);
	}
	
	// Pedestrian buttons are active high.
	pinMode(buttonPins[8], INPUT_PULLUP);
	
	// Add phases to the system.
	for (int i = 0; i < 9; i++) {
		trafficsys.phases.push_back(&phases[i]);
	}
	
	// Initialise pins for shift register.
	pinMode(SR_SCK, OUTPUT);
	pinMode(SR_RCK, OUTPUT);
	pinMode(SR_DAT, OUTPUT);
	
	// Lights test.
	// for (int i = 0; i < 9; i++) {
	// 	for (int x = 0; x < 3; x++) {
	// 		Color col = x == 0 ? Color::Green
	// 					: x == 1 ? Color::Yellow
	// 					: Color::Red;
	// 		phases[i].light->color = col;
	// 		for (int z = 0; z < 100; z++) {
	// 			updateLights();
	// 			shift_register_send(SR_DAT, SR_SCK, SR_RCK, SR_BITS, srBits);
	// 			delay(10);
	// 		}
	// 	}
	// }
	
}

void loop() {
	// Perform traffic logic.
	trafficsys.update();
	
	// Update lights accordingly.
	updateLights();
	
	// Update pedestrian lights.
	for (int i = 0; i < 8; i++) {
		// Pedestrians start at index 24, and go red, green, red, etc.
		int offset = 24 + 2 * i;
		srBits[offset]   = pedestianRed;
		srBits[offset+1] = pedestianGreen;
	}
	
	// Send bits to shift register.
	shift_register_send(SR_DAT, SR_SCK, SR_RCK, SR_BITS, srBits);
	
	// Check for traffic sensors (active low).
	for (int i = 0; i < 8; i++) {
		if (!digitalRead(buttonPins[i])) {
			phases[i].notify();
		}
	}
	
	// Check for pedestrian sensors (active high).
	if (digitalRead(buttonPins[8])) {
		phases[8].notify();
	}
	
	delay(10);
}