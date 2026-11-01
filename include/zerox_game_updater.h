#pragma once

#include <glm/gtc/matrix_transform.hpp>

#include "zerox_game_background.h"
#include "zerox_game_collider.h"
#include "zerox_game_obstacle.h"

namespace ZeroXGame
{
	inline void updateBackground(ZeroXGame::Background& background, double deltaTime)
	{
		ZeroXBridge::RenderEntity& renderEntity = background.entity.renderEntity;

		renderEntity.transform.localScale	= glm::scale(glm::mat4(1.0f), { 33.0f, 16.875f, 1.0f });
		renderEntity.material.uvOffset		+= static_cast<float>(deltaTime) * background.scrollSpeed;
		renderEntity.layer					= background.layer;
	}

	inline void updateObstacle(ZeroXGame::Obstacle& obstacle, double deltaTime)
	{
		ZeroXBridge::Material& material = obstacle.gameEntity.renderEntity.material;

		material.tileSizeX = static_cast<uint64_t>(obstacle.tileSize.x);
		material.tileSizeY = static_cast<uint64_t>(obstacle.tileSize.y);
	}

	inline void updateCollider(ZeroXGame::Collider& collider)
	{
		ZeroXBridge::RenderEntity& renderEntity = collider.gameEntity.renderEntity;

		ZeroXBridge::Transform& transform = renderEntity.transform;

		transform					= *collider.target;
		transform.localScale		= glm::scale(glm::mat4(1.0f), { collider.size.x, collider.size.y, 1.0f });
		renderEntity.material.color = glm::vec4{ 0.0f, 1.0f, 0.0f, 1.0f };
	}
}
