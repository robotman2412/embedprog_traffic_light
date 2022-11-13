#include "main.hpp"

#define BTN_UP      2
#define BTN_DOWN   17
#define BTN_LEFT   16
#define BTN_RIGHT   4
#define BTN_CENTER 15

#define HAL_PIN 5

#define SR_SCK 13
#define SR_RCK 21
#define SR_DAT 18

// Light lightA(13, LIGHT_YELLOW_EMU, 21);
// Light lightB(14, LIGHT_YELLOW_EMU, 12);

bool srBits[6];

Light lightA(srBits + 0, srBits + 1);
Light lightB(srBits + 2, srBits + 3);
Light lightC(srBits + 4, srBits + 5);

Detector detA;
Detector detB;
Detector detC;

Phase phaseA;
Phase phaseB;
Phase phaseC;

System trafficsys;

void setup() {
	pinMode(25, OUTPUT);
	digitalWrite(25, 0);
	
	startPwmTask();
	
	// The test LIGHT.
	registerLight(&lightA);
	registerLight(&lightB);
	registerLight(&lightC);
	
	detA = Detector(0, BTN_LEFT, FALLING);
	detB = Detector(0, BTN_RIGHT, FALLING);
	detC = Detector(0, BTN_DOWN, FALLING);
	
	// The test SYSTEM.
	phaseA = Phase("A", &lightA);
	phaseA.exclusive = 0x02;
	phaseA.clearTime = 1000;
	phaseA.addDetector(detA);
	
	phaseB = Phase("B", &lightB);
	phaseB.exclusive = 0x05;
	phaseB.clearTime = 1000;
	phaseB.addDetector(detB);
	
	phaseC = Phase("C", &lightC);
	phaseC.exclusive = 0x02;
	phaseC.clearTime = 1000;
	phaseC.addDetector(detC);
	
	// Add phases to the SYSTEM.
	trafficsys.phases.push_back(&phaseA);
	trafficsys.phases.push_back(&phaseB);
	trafficsys.phases.push_back(&phaseC);
	
	// Initialise.
	lightA.color  = Color::Red;
	lightB.color  = Color::Green;
	phaseB.onSice = millis();
	lightC.color  = Color::Red;
	trafficsys.currentPhase = 0x02;
	
	// DEFAULT is A is GREEN.
	trafficsys.defaultPhase = 0x01;
	
	pinMode(HAL_PIN, INPUT_PULLUP);
	
	pinMode(SR_SCK, OUTPUT);
	pinMode(SR_RCK, OUTPUT);
	pinMode(SR_DAT, OUTPUT);
}

void loop() {
	trafficsys.update();
	shift_register_send(SR_DAT, SR_SCK, SR_RCK, 6, srBits);
	delay(50);
}