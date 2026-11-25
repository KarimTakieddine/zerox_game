#include "zerox_game_obstacle.h"

namespace ZeroXGame
{
	void updateObstacle(Obstacle* obstacle, double deltaTime)
	{

	}

	void updateMovingPlatform(MovingPlatform* movingPlatform, double deltaTime)
	{
		movingPlatform->timer += static_cast<float>(deltaTime);
	}

	void fixedUpdateObstacle(Obstacle* obstacle, double deltaTime)
	{

	}

	void fixedUpdateMovingPlatform(MovingPlatform* movingPlatform, double deltaTime)
	{
		if (movingPlatform->switchTime > 0.0f && movingPlatform->timer >= movingPlatform->switchTime)
		{
			movingPlatform->velocity	= -movingPlatform->velocity;
			movingPlatform->timer		= 0;
		}

		movingPlatform->obstacle.gameEntity.physicsState.currentPosition += movingPlatform->velocity * static_cast<float>(deltaTime);
	}
}