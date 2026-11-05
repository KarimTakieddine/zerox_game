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

	inline void updateBackground(ZeroXGame::Background* background, double deltaTime)
	{
		ZeroXBridge::RenderEntity& renderEntity = background->entity.renderEntity;

		renderEntity.transform.localScale	= glm::scale(glm::mat4(1.0f), { 33.0f, 16.875f, 1.0f });
		renderEntity.material.uvOffset		+= static_cast<float>(deltaTime) * background->scrollSpeed;
		renderEntity.layer					= background->layer;
	}
}
