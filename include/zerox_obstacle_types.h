#pragma once

#include "zerox_game_collider.h"
#include "zerox_game_entity.h"

namespace ZeroXGame
{
	struct alignas(16) Obstacle
	{
		GameEntity gameEntity;
		Collider2D collider;
	};

	struct alignas(16) MovingPlatform
	{
		Obstacle obstacle;
		glm::vec2 velocity	{ 0.0f, 0.0f };
		float timer			{ 0.0f };
		float switchTime	{ 0.0f };
	};
}
