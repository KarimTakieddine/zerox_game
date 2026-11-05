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

	inline void updateObstacle(ZeroXGame::Obstacle* obstacle, double deltaTime)
	{
		ZeroXBridge::Material& material = obstacle->gameEntity.renderEntity.material;

		material.tileSizeX = static_cast<uint64_t>(obstacle->tileSize.x);
		material.tileSizeY = static_cast<uint64_t>(obstacle->tileSize.y);
	}
}
