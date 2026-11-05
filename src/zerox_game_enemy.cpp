#include "zerox_game_enemy.h"

namespace ZeroXGame
{
	void updateEnemy(Enemy* enemy, double deltaTime)
	{

	}

	void fixedUpdateEnemy(Enemy* enemy, double deltaTime)
	{
		enemy->gameEntity.physicsState.currentPosition.x += static_cast<float>(0.90 * deltaTime);
	}
}