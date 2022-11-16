
#pragma once

#include "light.hpp"
#include <vector>



class System;
class Phase;

class Phase {
	public:
		// The light that will be turned on by this phase.
		Light      *light;
		// The parent traffic system.
		System     *parent;
		
		// The amount of priority this phase has in the parent system.
		float       priority;
		// The bitmap of phases that this is incompatible with.
		uint32_t    exclusive;
		
		// The time at which this phase started waiting, or 0 if not currently waiting.
		uint64_t    waitingSince;
		// The amount of time it takes for traffic to clear this phase.
		uint64_t    clearTime;
		// Since when this was turned on.
		uint64_t    onSince;
		// How long this needs to remain on.
		uint64_t    onTime;
		
		// Name for this phase.
		const char *name;
		
		Phase();
		// Make a phase.
		Phase(const char *name, Light *light);
		
		// Something just arrived and is waiting for this phase.
		void notify();
};

class System {
	public:
		// All phases that affect the system.
		std::vector<Phase *> phases;
		// The bitmap of the next phases to turn on, or 0 if none.
		uint32_t nextPhase;
		// The bitmap of phases that are currently on.
		uint32_t currentPhase;
		// How long to wait for turning on the next phase.
		uint64_t changeTime;
		
		System();
		
		// The function that reolves which lights to turn on next.
		void update();
};
