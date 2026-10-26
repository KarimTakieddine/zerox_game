#include <glm/gtc/matrix_transform.hpp>

#include "zerox_game_updater.h"

namespace ZeroXGame
{
	void updateBackground(ZeroXGame::Background& background)
	{
		background.entity.transform.localScale = glm::scale(glm::mat4(1.0f), { 33.0f, 16.875f, 1.0f });
	}

	void updateCollider(ZeroXGame::Collider& collider)
	{
		if (!collider.target)
		{
			return;
		}

		collider.gameEntity.transform				= *collider.target;
		collider.gameEntity.transform.localScale	= glm::scale(
			glm::mat4(1.0f),
			{ collider.size.x, collider.size.y, 1.0f });
		collider.gameEntity.material.color			= glm::vec4{ 0.0f, 1.0f, 0.0f, 1.0f };
	}
}