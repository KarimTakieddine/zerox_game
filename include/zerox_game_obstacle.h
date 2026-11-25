#pragma once

#include "zerox_obstacle_types.h"

namespace ZeroXGame
{
	void updateObstacle(Obstacle* obstacle, double deltaTime);
	void updateMovingPlatform(MovingPlatform* movingPlatform, double deltaTime);

	void fixedUpdateObstacle(Obstacle* obstacle, double deltaTime);
	void fixedUpdateMovingPlatform(MovingPlatform* movingPlatform, double deltaTime);
}
