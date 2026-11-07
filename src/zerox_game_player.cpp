#include "zerox_game_player.h"

namespace ZeroXGame
{
	void updatePlayer(Player* player, double deltaTime)
	{

	}

	void fixedUpdatePlayer(Player* player, double deltaTime)
	{
		player->gameEntity.physicsState.currentPosition.y -= static_cast<float>(10.0f * deltaTime);
	}
}