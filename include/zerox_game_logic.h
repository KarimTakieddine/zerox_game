#pragma once

#include "zerox_game_memory_view.h"

namespace ZeroXGame
{
	// TODO(Karim): Do we need an input snapshot here?

	void update(const LevelView* level, double deltaTime, const ZeroXBridge::Input* input, const Animation* animations);
	void fixedUpdate(const LevelView* level, double deltaTime, const ZeroXBridge::Input* input);
	void lerpPhysics(const LevelView* level, float alpha);
	void step(const LiveMemoryView& memory, const ZeroXBridge::Input* input, const Animation* animations, double deltaTime, double fixedDeltaTime, double* physicsAccumulator);
}
