#include "main.hpp"

#define BTN_UP      2
#define BTN_DOWN   17
#define BTN_LEFT   16
#define BTN_RIGHT   4
#define BTN_CENTER 15

Light lightA(13, LIGHT_YELLOW_EMU, 21);
Light lightB(14, LIGHT_YELLOW_EMU, 12);

Detector detA;
Detector detB;

Phase phaseA;
Phase phaseB;

System trafficsys;

void setup() {
	pinMode(25, OUTPUT);
	digitalWrite(25, 0);
	
	startPwmTask();
	
	// The test LIGHT.
	registerLight(&lightA);
	registerLight(&lightB);
	
	detA = Detector(0, BTN_LEFT, FALLING);
	detB = Detector(0, BTN_RIGHT, FALLING);
	
	// The test SYSTEM.
	phaseA = Phase("A", &lightA, &detA);
	phaseA.exclusive = 0x02;
	phaseA.clearTime = 1000;
	detA.phase = &phaseA;
	
	phaseB = Phase("A", &lightB, &detB);
	phaseB.exclusive = 0x01;
	phaseB.clearTime = 1000;
	detB.phase = &phaseB;
	
	
	// Add phases to the SYSTEM.
	trafficsys.phases.push_back(&phaseA);
	trafficsys.phases.push_back(&phaseB);
	
	// Initialise at A is GREEN, B is RED.
	lightA.color  = Color::Red;
	phaseA.onSice = millis();
	lightB.color  = Color::Green;
	trafficsys.currentPhase = 0x02;
	
	// DEFAULT is A is GREEN.
	trafficsys.defaultPhase = 0x01;
	
}

void loop() {
	trafficsys.update();
}