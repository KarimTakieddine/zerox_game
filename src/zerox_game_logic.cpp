#include <algorithm>

#include "zerox_game_animation.h"
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

	struct AABB
	{
		glm::vec2 topLeft		{ 0.0f, 0.0f };
		glm::vec2 topRight		{ 0.0f, 0.0f };
		glm::vec2 bottomRight	{ 0.0f, 0.0f };
		glm::vec2 bottomLeft	{ 0.0f, 0.0f };
	};

	constexpr AABB getAABB(ZeroXGame::PhysicsState& state, const ZeroXGame::Collider* collider)
	{
		const glm::vec2& colliderSize = collider->size;

		const float halfWidth	= colliderSize.x * 0.5f;
		const float halfHeight	= colliderSize.y * 0.5f;

		const glm::vec2& center = state.currentPosition + collider->offset;

		return {
			.topLeft		= center + glm::vec2{ -halfWidth, halfHeight },
			.topRight		= center + glm::vec2{ halfWidth, halfHeight },
			.bottomRight	= center + glm::vec2{ halfWidth, -halfHeight },
			.bottomLeft		= center + glm::vec2{ -halfWidth, -halfHeight }
		};
	}

	struct AxisProjections
	{
		float topLeft		{ 0.0f };
		float topRight		{ 0.0f };
		float bottomRight	{ 0.0f };
		float bottomLeft	{ 0.0f };
	};

	constexpr AxisProjections getAxisProjections(const AABB& aabb, const glm::vec2& axis)
	{
		return {
			.topLeft		= glm::dot(aabb.topLeft, axis),
			.topRight		= glm::dot(aabb.topRight, axis),
			.bottomRight	= glm::dot(aabb.bottomLeft, axis),
			.bottomLeft		= glm::dot(aabb.bottomRight, axis)
		};
	}

	struct AxisShadow
	{
		float min;
		float max;
	};

	constexpr AxisShadow getAxisShadow(const AxisProjections& projections)
	{
		const float projectionList[4] = {
			projections.topLeft,
			projections.topRight,
			projections.bottomRight,
			projections.bottomLeft
		};

		auto beginIt	= std::begin(projectionList);
		auto endIt		= std::end(projectionList);

		return {
			*std::min_element(beginIt, endIt),
			*std::max_element(beginIt, endIt)
		};
	}

	constexpr void resolveCollisions(ZeroXGame::PhysicsState& lhsState, ZeroXGame::PhysicsState& rhsState, const ZeroXGame::Collider* lhs, const ZeroXGame::Collider* rhs)
	{
		AABB lhsAABB = getAABB(lhsState, lhs);
		AABB rhsAABB = getAABB(rhsState, rhs);

		constexpr size_t axisCount{ 2 };

		const glm::vec2 axes[axisCount] = {
			{ 1.0f, 0.0f },
			{ 0.0f, 1.0f }
		};

		bool colliding{ true };

		float overlaps[axisCount << 1](0.0f);
		size_t overlapIndex{ 0 };

		for (size_t i = 0; i < axisCount; ++i)
		{
			const glm::vec2 axis = axes[i];

			const AxisProjections lhsProjections = getAxisProjections(lhsAABB, axis);
			const AxisProjections rhsProjections = getAxisProjections(rhsAABB, axis);

			const AxisShadow lhsShadow = getAxisShadow(lhsProjections);
			const AxisShadow rhsShadow = getAxisShadow(rhsProjections);

			if (lhsShadow.min > rhsShadow.max || lhsShadow.max < rhsShadow.min)
			{
				colliding = false;
				break;
			}

			overlaps[overlapIndex++] = glm::abs(lhsShadow.max - rhsShadow.min);
			overlaps[overlapIndex++] = glm::abs(rhsShadow.max - lhsShadow.min);
		}

		if (colliding)
		{
			lhsState.currentPosition +=
				*std::min_element(std::begin(overlaps), std::end(overlaps)) * rhs->normal;
		}
	}
}

namespace ZeroXGame
{
	void update(const GameMemory& memory, double deltaTime, const ZeroXBridge::Input* input)
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
			updatePlayer(players.data() + i, deltaTime, input, memory.animations.data());
		}
	}

	void fixedUpdate(const GameMemory& memory, double deltaTime, const ZeroXBridge::Input* input)
	{
		const auto& obstacles	= memory.obstacles;
		const auto& enemies		= memory.enemies;
		const auto& players		= memory.players;
		const auto& colliders	= memory.colliders;

		for (size_t i = 0; i < obstacles.size(); ++i)
		{
			auto* obstacle = obstacles.data() + i;

			::savePhysicsState(obstacle->gameEntity.physicsState);
			fixedUpdateObstacle(obstacle, deltaTime);
		}

		for (size_t i = 0; i < enemies.size(); ++i)
		{
			auto* enemy = enemies.data() + i;

			::savePhysicsState(enemy->gameEntity.physicsState);
			fixedUpdateEnemy(enemy, deltaTime);

			// TODO(Karim): Resolve collisions against obstacles
		}

		for (size_t i = 0; i < players.size(); ++i)
		{
			auto* player = players.data() + i;

			::savePhysicsState(player->gameEntity.physicsState);
			fixedUpdatePlayer(player, deltaTime, input);

			const Collider* playerCollider = colliders.data() + i;

			for (size_t j = 0; j < obstacles.size(); ++j)
			{
				auto* obstacle = obstacles.data() + j;

				const Collider* obstacleCollider =
					colliders.data() + enemies.size() + players.size() + j;

				::resolveCollisions(
					player->gameEntity.physicsState,
					obstacle->gameEntity.physicsState,
					playerCollider,
					obstacleCollider);
			}
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

		*physicsAccumulator += deltaTime;

		const ZeroXBridge::Input* input = memory.input.data();

		auto& accumulator = *physicsAccumulator;
		while (accumulator >= fixedDeltaTime)
		{
			fixedUpdate(memory, fixedDeltaTime, input);

			accumulator -= fixedDeltaTime;
		}

		lerpPhysics(memory, static_cast<float>(accumulator / fixedDeltaTime));

		const auto& colliders = memory.colliders;
		for (size_t i = 0; i < colliders.size(); ++i)
		{
			updateCollider(colliders.data() + i);
		}

		update(memory, deltaTime, input);
	}
}