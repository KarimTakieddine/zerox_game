#pragma once

#include "zerox_game_entity.h"

namespace ZeroXGame
{
	struct alignas(16) Obstacle
	{
		GameEntity gameEntity;
		glm::vec2 tileSize;
		glm::vec2 scale;
	};
}
