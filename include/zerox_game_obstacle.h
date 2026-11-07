#pragma once

#include "zerox_game_entity.h"
#include "zerox_game_collider.h"

namespace ZeroXGame
{
	struct alignas(16) Obstacle
	{
		GameEntity gameEntity;
	};

	inline void fixedUpdateObstacle(ZeroXGame::Obstacle* obstacle, double deltaTime)
	{

	}

	inline void updateObstacle(ZeroXGame::Obstacle* obstacle, double deltaTime)
	{

	}
}
