#include <zerox_input.h>

#include "zerox_game_animation.h"
#include "zerox_game_player.h"

namespace ZeroXGame
{
	void updatePlayer(Player* player, double deltaTime, const ZeroXBridge::Input* input)
	{

	}

	void fixedUpdatePlayer(Player* player, double deltaTime, const ZeroXBridge::Input* input)
	{
		player->gameEntity.physicsState.currentPosition.y -= static_cast<float>(10.0f * deltaTime);
	}
}