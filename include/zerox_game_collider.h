#pragma once

#include <glm/vec2.hpp>

#include "zerox_game_entity.h"

namespace ZeroXGame
{
	struct alignas(16) Collider
	{
		GameEntity gameEntity;
		glm::vec2 size{ 1.0f, 1.0f };
		Transform* target;
	};
}