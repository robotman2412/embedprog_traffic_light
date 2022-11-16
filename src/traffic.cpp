
#include "traffic.hpp"



Phase::Phase() {
	this->light    = NULL;
	this->name     = "";
	parent         = NULL;
	priority       = 1;
	exclusive      = 0;
	waitingSince   = 0;
}

// Make a phase.
Phase::Phase(const char *name, Light *light) {
	this->light    = light;
	this->name     = name;
	parent         = NULL;
	priority       = 1;
	exclusive      = 0;
	waitingSince   = 0;
	clearTime      = 2000;
	onTime         = 2000;
}

// Traffic just arrived and is waiting for this phase.
void Phase::notify() {
	uint64_t now = millis();
	
	// If already green, skip.
	if (light->color == Color::Green) return;
	
	// Update waiting time.
	if (!waitingSince || now < waitingSince) {
		waitingSince = now;
	}
}



System::System() {
	currentPhase = 0;
	changeTime   = 0;
	nextPhase    = 0;
}

// Apply the scheduled change.
void System::applyChange() {
	uint64_t now = millis();
	
	// Final changening of the PHASES.
	for (size_t i = 0; i < phases.size(); i++) {
		// Should the phase be on?
		bool active = (nextPhase >> i) & 1;
		
		// Set to red or green accordingly.
		phases[i]->light->color = active ? Color::Green : Color::Red;
		
		// Clear waiting time, if applicable.
		if (active) {
			phases[i]->waitingSince = 0;
			phases[i]->onSince = now;
		}
	}
	// Mark system as done changing.
	changeTime   = 0;
	// Update the active phases to the target.
	currentPhase = nextPhase;
	
}

// Turn traffic lights yellow when scheduled.
void System::turnLightsYellow() {
	uint64_t now = millis();
	
	// The amount of time left until the scheduled lights turn on.
	uint64_t timeLeft = changeTime - now;
	
	// Turning yellow and then red logic.
	for (size_t i = 0; i < phases.size(); i++) {
		// Whether the phase is on right now.
		bool pre  = (currentPhase >> i) & 1;
		// Whether the phase should be on afterwards.
		bool post = (nextPhase    >> i) & 1;
		
		// Whether the phase should turn off.
		bool turnsOff = pre && !post;
		
		// Turn yellow first.
		bool yellow = turnsOff && timeLeft <= phases[i]->clearTime*2;
		// And red afterwards.
		bool red    = turnsOff && timeLeft <= phases[i]->clearTime;
		
		// Update colors in the lights.
		if (red)         phases[i]->light->color = Color::Red;
		else if (yellow) phases[i]->light->color = Color::Yellow;
	}
}

// Decide what new change to schedule, if any.
void System::scheduleChange() {
	uint64_t now = millis();
	
	// The following determines how to change phases.
	uint32_t map = 0;
	
	// Add in everything that is required to stay on.
	for (size_t i = 0; i < phases.size(); i++) {
		if (currentPhase & (1 << i)) {
			uint64_t onTime = now - phases[i]->onSince;
			if (onTime < phases[i]->onTime) {
				map |= 1 << i;
			}
		}
	}
	
	// The highest priority found.
	float maxPrio =  0;
	// Index of the found phase, or -1 if none.
	int   found   = -1;
	
	// Look for possible things to schedule.
	for (size_t i = 0; i < phases.size(); i++) {
		// Priority is a function of time.
		float prio  = phases[i]->waitingSince
				? (now - phases[i]->waitingSince) * phases[i]->priority
				: 0;
		bool compatible = !(phases[i]->exclusive & map);
		if (prio > maxPrio && compatible) {
			// The highest priority phase is always included.
			maxPrio = prio;
			found   = i;
		}
	}
	
	// If nothing is found, don't bother with looking for more.
	if (found == -1) return;
	
	// This is a bitmap of the phases to turn on.
	map |= 1 << found;
	
	// Continually add more compatible phases.
	do {
		// Look again for the highest priority thing.
		found = -1;
		maxPrio = 0;
		for (size_t i = 0; i < phases.size(); i++) {
			// Exclude phases already in the set.
			if (~map & (1 << i)) {
				// Priority is a function of time.
				float prio  = phases[i]->waitingSince ? (now - phases[i]->waitingSince) * phases[i]->priority : 0;
				bool compatible = !(phases[i]->exclusive & map);
				if (prio > maxPrio && compatible) {
					// The highest remaining priority phase is added.
					maxPrio = prio;
					found   = i;
				}
			}
		}
		
		// If found is not -1, then there is a phase to add.
		if (found != -1) {
			// Add the phase to the bitmap of phases to turn on.
			map |= 1 << found;
		}
		
	} while (found != -1);
	
	// Keep all phases in current which are compatible.
	for (int i = 0; i < phases.size(); i++) {
		// If phase is active...
		if (currentPhase & (1 << i)) {
			// And compatible...
			if (!(phases[i]->exclusive & map)) {
				// Add it to the map!
				map |= 1 << i;
			}
		}
	}
	
	// The highest found amount of time required for traffic to exit.
	uint64_t clearTime = 0;
	// The phases that are to be turned off later.
	uint32_t removed   = currentPhase & ~map;
	// Then find the time required for clearing of active phases.
	for (size_t i = 0; i < phases.size(); i++) {
		// If the phase is in the map...
		if (removed & (1 << i)) {
			// Include it's clearing time.
			if (phases[i]->clearTime > clearTime) {
				clearTime = phases[i]->clearTime;
			}
		}
	}
	
	// Finally, schedule the phase change.
	nextPhase  = map;
	changeTime = millis() + clearTime * 2;
}

// The main loop for deciding which lights do what.
void System::update() {
	uint64_t now = millis();
	
	// Check for phases to change.
	if (nextPhase != currentPhase && changeTime != 0 && changeTime <= now) {
		// Apply the scheduled phase change.
		applyChange();
		
	} else if (nextPhase != currentPhase && changeTime > now) {
		// Turning yellow and red of the phases.
		turnLightsYellow();
		
	} else {
		// Try to schedule more phase changes.
		scheduleChange();
	}
}
