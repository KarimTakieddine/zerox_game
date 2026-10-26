#pragma once

#include <glm/vec2.hpp>

#include "zerox_game_entity.h"

namespace ZeroXGame
{
	struct alignas(16) Background
	{
		GameEntity entity;
		glm::vec2 scrollSpeed	{ 0.0f, 0.0f };
		uint64_t layer			{ 0 };
	};
}
