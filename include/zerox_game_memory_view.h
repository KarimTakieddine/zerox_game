#pragma once

#include <span>

#include <zerox_input.h>

#include "zerox_enemy_types.h"
#include "zerox_game_animation.h"
#include "zerox_game_background.h"
#include "zerox_game_memory_header.h"
#include "zerox_game_player.h"
#include "zerox_obstacle_types.h"

namespace ZeroXBridge
{
	struct RenderDescriptor;
	struct EntitySection;
};

namespace ZeroXGame
{
	struct alignas(16) LevelView
	{
		std::span<Background> backgrounds;
		std::span<Obstacle> obstacles;
		std::span<MovingPlatform> movingPlatforms;
		std::span<Skull> skulls;
		std::span<Mouns> mounsList;
		std::span<Player> players;
	};

	struct alignas(16) GameMemoryView
	{
		std::span<ZeroXBridge::Input> input;
		std::span<LevelView> levels;
		std::span<Animation> animations;
	};

	struct alignas(16) LiveMemoryView
	{
		std::span<LevelView> level;
		std::span<Collider2DEntity> colliderEntities;
		std::span<ZeroXBridge::EntitySection> entitySections;
	};

	inline constexpr size_t getByteSize(const LevelView* level)
	{
		return
			( level->backgrounds.size() * sizeof(Background) )			+
			( level->obstacles.size() * sizeof(Obstacle) )				+
			( level->movingPlatforms.size() * sizeof(MovingPlatform) )	+
			( level->skulls.size() * sizeof(Skull) )					+
			( level->mounsList.size() * sizeof(Mouns) )					+
			( level->players.size() * sizeof(Player) );
	}

	inline constexpr size_t getByteSize(const Animation* animation)
	{
		return ( animation->frameCount * sizeof(AnimationFrame) );
	}

	inline constexpr size_t getReservedByteSize(const GameMemoryView& view)
	{
		constexpr size_t memoryHeaderSize = sizeof(GameMemoryHeader);

		size_t result{ 0 };

		result += memoryHeaderSize + ( view.input.size() * sizeof(ZeroXBridge::Input) );

		const auto levels		= view.levels;
		const size_t levelCount = levels.size();

		result += memoryHeaderSize + ( levelCount * sizeof(LevelView) );

		const auto animations		= view.animations;
		const size_t animationCount = animations.size();

		result += memoryHeaderSize + ( animationCount * sizeof(Animation) );

		for (size_t i = 0; i < levelCount; ++i)
		{
			result += getByteSize(levels.data() + i);
		}

		for (size_t i = 0; i < animationCount; ++i)
		{
			result += getByteSize(animations.data() + i);
		}

		return result;
	}
}
