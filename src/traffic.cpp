
#include "traffic.hpp"



// Callback used for interrupts in detect button thing.
static void detectorButtonCallback(void *ptr) {
	Detector *detector = (Detector *) ptr;
	if (detector->phase) detector->phase->notify(detector->distance);
}



Detector::Detector() {
	buttonPin = -1;
	buttonAttached = false;
}

// Make a button detector.
Detector::Detector(uint64_t distance, int buttonPin, int buttonMode) {
	this->distance   = distance;
	this->buttonPin  = buttonPin;
	this->buttonMode = buttonMode;
	buttonAttached   = false;
}

// Removal of detector.
Detector::~Detector() {
	if (buttonAttached) {
		buttonAttached = false;
		detachInterrupt(digitalPinToInterrupt(this->buttonPin));
	}
}

// Tell the THING that it is ATTACHED to a PHASE.
void Detector::attached() {
	pinMode(buttonPin, buttonMode == RISING ? INPUT_PULLDOWN : INPUT_PULLUP);
	attachInterruptArg(digitalPinToInterrupt(this->buttonPin), detectorButtonCallback, (void *) this, buttonMode);
}



Phase::Phase() {
	this->light    = NULL;
	this->name     = "";
	parent         = NULL;
	priority       = 1;
	exclusive      = 0;
	waitingSince   = 0;
	onSice         = 0;
}

Phase::Phase(Phase &other) {
	light     = other.light;
	name      = other.name;
	detectors = other.detectors;
}

// Make a phase.
Phase::Phase(const char *name, Light *light) {
	this->light    = light;
	this->name     = name;
	parent         = NULL;
	priority       = 1;
	exclusive      = 0;
	waitingSince   = 0;
	onSice         = 0;
}

// ADD a DETECTOR to this PHASE.
void Phase::addDetector(Detector detector) {
	// Add it to the list.
	detectors.push_back(detector);
	auto ptr = detectors.end() - 1;
	
	// Install the detector.
	ptr->phase = this;
	ptr->attached();
}

// Something just arrived and is waiting for this phase.
void Phase::notify(uint64_t distance) {
	uint64_t nextTime = millis() + distance;
	if (!waitingSince || nextTime < waitingSince) {
		waitingSince = nextTime;
		if (this->waitingSince) digitalWrite(25, 1);
	}
}



System::System() {
	defaultPhase = 0;
	currentPhase = 0;
	changeTime   = 0;
	nextPhase    = 0;
}

// The handlage looper for handlage ALL THE THINGS!
void System::update() {
	uint64_t now = millis();
	
	// Check for phases to change.
	if (nextPhase != currentPhase && changeTime && changeTime <= now) {
		// Final changening of the PHASES.
		for (size_t i = 0; i < phases.size(); i++) {
			// Is it to activate?
			bool active = (nextPhase >> i) & 1;
			// Set to red or green accordingly.
			phases[i]->light->color = active ? Color::Green : Color::Red;
			phases[i]->onSice       = active ? now : 0;
			phases[i]->waitingSince = active ? 0 : phases[i]->waitingSince;
		}
		changeTime   = 0;
		currentPhase = nextPhase;
		
	} else if (nextPhase != currentPhase && changeTime > now) {
		// Intermediate changening of the PHASES.
		uint64_t left = changeTime - now;
		for (size_t i = 0; i < phases.size(); i++) {
			// Determine changes.
			bool pre  = (currentPhase >> i) & 1;
			bool post = (nextPhase    >> i) & 1;
			// If it should turn off
			bool yellow = pre && !post && left <= phases[i]->clearTime*2;
			bool red    = pre && !post && left <= phases[i]->clearTime;
			// Then ORANGE it.
			if (red)         phases[i]->light->color = Color::Red;
			else if (yellow) phases[i]->light->color = Color::Yellow;
		}
		
	} else {
		// Look for possible things to schedule.
		float maxPrio =  0;
		int   found   = -1;
		for (size_t i = 0; i < phases.size(); i++) {
			// Priority is a function of time.
			float prio  = phases[i]->waitingSince ? (now - phases[i]->waitingSince) * phases[i]->priority : 0;
			if (prio > maxPrio) {
				// The highest priority phase is always included.
				maxPrio = prio;
				found   = i;
			}
		}
		
		// Continually add more compatible phases.
		if (found == -1) return;
		uint32_t map = 1 << found;
		do {
			// Look again for the highest priority thing.
			found = -1;
			maxPrio = 0;
			for (size_t i = 0; i < phases.size(); i++) {
				// Exclude phases already in the set.
				if (~map & (1 << i)) {
					// Priority is a function of time.
					float prio  = phases[i]->waitingSince ? (now - phases[i]->waitingSince) * phases[i]->priority : 0;
					if (prio > maxPrio) {
						// The highest remaining priority phase is added.
						maxPrio = prio;
						found   = i;
					}
				}
			}
			if (found != -1) map |= 1 << found;
			
		} while (found != -1);
		
		// Then find the time required for clearing of active phases.
		uint64_t clearTime = 0;
		for (size_t i = 0; i < phases.size(); i++) {
			// If the phase is in the map...
			if (map & (1 << i)) {
				// Include it's clearing time.
				if (phases[i]->clearTime > clearTime) {
					clearTime = phases[i]->clearTime;
				}
			}
		}
		
		// Finally, schedule the thing.
		nextPhase  = map;
		changeTime = millis() + clearTime * 2;
	}
}
