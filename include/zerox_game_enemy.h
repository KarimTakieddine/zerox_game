#pragma once

#include "zerox_game_entity.h"

namespace ZeroXGame
{
	struct alignas(16) Enemy
	{
		GameEntity gameEntity;
	};

	void updateEnemy(Enemy* enemy, double deltaTime);
	void fixedUpdateEnemy(Enemy* enemy, double deltaTime);
}
