#pragma once

#include "zerox_game_memory.hpp"

namespace ZeroXGame
{
	// TODO(Karim): Do we need an input snapshot here?

	void update(const GameMemory& memory, double deltaTime);
	void fixedUpdate(const GameMemory& memory, double deltaTime);
	void lerpPhysics(const GameMemory& memory, float alpha);
	void step(const GameMemory& memory, double deltaTime, double fixedDeltaTime, double* physicsAccumulator);
}
