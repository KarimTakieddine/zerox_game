#pragma once

#include "zerox_game_entity.h"
#include "zerox_game_collider.h"

namespace ZeroXGame
{
	struct alignas(16) Skull
	{
		GameEntity gameEntity;
		Collider2D collider;
		glm::vec2 cycleSpeed{ 0.0f, 0.0f };
		double cycleTime	{ 0.0 };
	};

	struct alignas(16) Mouns
	{
		GameEntity gameEntity;
		Collider2D collider;
		glm::vec2 direction	{ 0.0f, 0.0f };
		double speed		{ 0.0 };
	};
}
