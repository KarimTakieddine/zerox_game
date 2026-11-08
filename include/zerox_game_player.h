#pragma once

#include "zerox_game_entity.h"

namespace ZeroXBridge { struct Input; }

namespace ZeroXGame
{
	struct Player
	{
		GameEntity gameEntity;
	};

	void updatePlayer(Player* player, double deltaTime, const ZeroXBridge::Input* input);
	void fixedUpdatePlayer(Player* player, double deltaTime, const ZeroXBridge::Input* input);
}
