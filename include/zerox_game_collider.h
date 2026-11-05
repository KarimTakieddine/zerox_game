#pragma once

#include <glm/vec2.hpp>

#include "zerox_game_entity.h"

namespace ZeroXGame
{
	struct alignas(16) Collider
	{
		GameEntity gameEntity;
		glm::vec2 size{ 1.0f, 1.0f };
		ZeroXBridge::Transform* target;
	};

	inline void updateCollider(Collider* collider)
	{
		ZeroXBridge::RenderEntity& renderEntity = collider->gameEntity.renderEntity;

		ZeroXBridge::Transform& transform = renderEntity.transform;

		transform					= *collider->target;
		transform.localScale		= glm::scale(glm::mat4(1.0f), { collider->size.x, collider->size.y, 1.0f });
		renderEntity.material.color = glm::vec4{ 0.0f, 1.0f, 0.0f, 1.0f };
	}
}
