
#pragma once

#include "light.hpp"
#include <vector>



class System;
class Phase;

class Detector {
	public:
		// The phase to notify of detection.
		Phase    *phase;
		// The approximate distance in milliseconds of travel time.
		uint64_t  distance;
		
		// A button input.
		int       buttonPin;
		// The interrupt mode for button input.
		int       buttonMode;
		// Whether the button iterrupt was attached.
		bool      buttonAttached;
		
		Detector();
		// Make a button detector.
		Detector(uint64_t distance, int buttonPin, int buttonMode);
		// Tell the THING that it is ATTACHED to a PHASE.
		void attached();
		
		// Removal of detector.
		~Detector();
};

class Phase {
	public:
		// The light that will be turned on by this phase.
		Light      *light;
		// The detectors that queue this phase.
		std::vector<Detector> detectors;
		// The parent traffic system.
		System     *parent;
		// The amount of priority this phase has in the parent system.
		float       priority;
		// The bitmap of phases that this is incompatible with.
		uint32_t    exclusive;
		// The time at which this phase started waiting, or 0 if not currently waiting.
		uint64_t    waitingSince;
		// The time at which this phase was turned on.
		uint64_t    onSice;
		// The amount of time it takes for traffic to clear this phase.
		uint64_t    clearTime;
		// Name for this phase.
		const char *name;
		
		Phase();
		Phase(Phase &);
		// Make a phase.
		Phase(const char *name, Light *light);
		
		// ADD a DETECTOR to this PHASE.
		void addDetector(Detector detector);
		
		// Something just arrived and is waiting for this phase.
		void notify(uint64_t distance);
};

class System {
	public:
		// All phases that affect the system.
		std::vector<Phase *> phases;
		// The bitmap of the phases that are on by default, or 0 if none.
		uint32_t defaultPhase;
		// The bitmap of the next phases to turn on, or 0 if none.
		uint32_t nextPhase;
		// The bitmap of phases that are currently on.
		uint32_t currentPhase;
		// How long to wait for turning on the next phase.
		uint64_t changeTime;
		
		System();
		
		// The handlage looper for handlage ALL THE THINGS!
		void update();
};
