#pragma once

#include <span>

namespace ZeroXBridge
{
	struct Input;
	struct RenderDescriptor;
	struct EntitySection;
};

namespace ZeroXGame
{
	struct Animation;
	struct Background;
	struct Collider2DEntity;
	struct Level;
	struct Mouns;
	struct MovingPlatform;
	struct Obstacle;
	struct Player;
	struct Skull;

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
		std::span<LevelView> currentLevelView;
		std::span<Collider2DEntity> colliderEntities;
		std::span<ZeroXBridge::RenderDescriptor> renderDescriptors;
	};
}
