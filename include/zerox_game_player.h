#pragma once

#include "zerox_game_entity.h"

namespace ZeroXGame
{
	struct Player
	{
		GameEntity gameEntity;
	};

	void updatePlayer(Player* player, double deltaTime);
	void fixedUpdatePlayer(Player* player, double deltaTime);
}
