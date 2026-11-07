#pragma once

#include <glm/vec2.hpp>

#include "zerox_game_entity.h"

namespace ZeroXGame
{
	struct alignas(16) Collider
	{
		GameEntity gameEntity;
		glm::vec2 size	{ 1.0f, 1.0f };
		glm::vec2 offset{ 0.0f, 0.0f };
		glm::vec2 normal{ 0.0f, 0.0f };
		ZeroXBridge::Transform* target;
	};

	inline void updateCollider(Collider* collider)
	{
		collider->gameEntity.renderEntity.transform = *collider->target;

		// TODO(Karim): Account for offset!
	}
}
