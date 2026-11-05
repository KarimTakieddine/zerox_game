#include "zerox_game_logic.h"
#include "zerox_game_player.h"
#include "zerox_game_transform.h"

namespace
{
	void savePhysicsState(ZeroXGame::PhysicsState& state)
	{
		state.lastPosition = state.currentPosition;
	}

	void lerpPhysicsState(ZeroXGame::PhysicsState& state, ZeroXBridge::Transform& transform, float alpha)
	{
		const glm::vec2 lerpPosition =
			state.lastPosition * (1.0f - alpha) +
			state.currentPosition * alpha;

		ZeroXGame::setPosition(transform, lerpPosition);
	}
}

namespace ZeroXGame
{
	void update(const GameMemory& memory, double deltaTime)
	{
		const auto& backgrounds = memory.backgrounds;
		const auto& obstacles	= memory.obstacles;
		const auto& enemies		= memory.enemies;
		const auto& players		= memory.players;
		
		for (size_t i = 0; i < backgrounds.size(); ++i)
		{
			updateBackground(backgrounds.data() + i, deltaTime);
		}

		for (size_t i = 0; i < obstacles.size(); ++i)
		{
			updateObstacle(obstacles.data() + i, deltaTime);
		}

		for (size_t i = 0; i < enemies.size(); ++i)
		{
			updateEnemy(enemies.data() + i, deltaTime);
		}

		for (size_t i = 0; i < players.size(); ++i)
		{
			updatePlayer(players.data() + i, deltaTime);
		}
	}

	void fixedUpdate(const GameMemory& memory, double deltaTime)
	{
		const auto& obstacles	= memory.obstacles;
		const auto& enemies		= memory.enemies;
		const auto& players		= memory.players;

		for (size_t i = 0; i < obstacles.size(); ++i)
		{
			auto* obstacle = obstacles.data() + i;

			// fixedUpdateObstacle(obstacle, deltaTime);
			::savePhysicsState(obstacle->gameEntity.physicsState);
		}

		for (size_t i = 0; i < enemies.size(); ++i)
		{
			auto* enemy = enemies.data() + i;

			fixedUpdateEnemy(enemy, deltaTime);
			::savePhysicsState(enemy->gameEntity.physicsState);
		}

		for (size_t i = 0; i < players.size(); ++i)
		{
			auto* player = players.data() + i;

			fixedUpdatePlayer(player, deltaTime);
			::savePhysicsState(player->gameEntity.physicsState);
		}
	}

	void lerpPhysics(const GameMemory& memory, float alpha)
	{
		const auto& obstacles	= memory.obstacles;
		const auto& enemies		= memory.enemies;
		const auto& players		= memory.players;

		for (size_t i = 0; i < obstacles.size(); ++i)
		{
			auto* obstacle = obstacles.data() + i;

			lerpPhysicsState(
				obstacle->gameEntity.physicsState,
				obstacle->gameEntity.renderEntity.transform,
				alpha);
		}

		for (size_t i = 0; i < enemies.size(); ++i)
		{
			auto* enemy = enemies.data() + i;

			lerpPhysicsState(
				enemy->gameEntity.physicsState,
				enemy->gameEntity.renderEntity.transform,
				alpha);
		}

		for (size_t i = 0; i < players.size(); ++i)
		{
			auto* player = players.data() + i;

			lerpPhysicsState(
				player->gameEntity.physicsState,
				player->gameEntity.renderEntity.transform,
				alpha);
		}
	}

	void step(const GameMemory& memory, double deltaTime, double fixedDeltaTime, double* physicsAccumulator)
	{
		assert(fixedDeltaTime != 0.0);

		update(memory, deltaTime);

		*physicsAccumulator += deltaTime;

		auto& accumulator = *physicsAccumulator;
		while (accumulator >= fixedDeltaTime)
		{
			fixedUpdate(memory, fixedDeltaTime);

			accumulator -= fixedDeltaTime;
		}

		lerpPhysics(memory, static_cast<float>(accumulator / fixedDeltaTime));

		const auto& colliders = memory.colliders;
		for (size_t i = 0; i < colliders.size(); ++i)
		{
			updateCollider(colliders.data() + i);
		}
	}
}