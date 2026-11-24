#pragma once

namespace ZeroXGame
{
	struct Background;
	struct Player;
	struct Skull;
	struct Mouns;
	struct Obstacle;
	struct MovingPlatform;

	struct alignas(16) Level
	{
		Background* backgrounds			{ nullptr };
		Player* players					{ nullptr };
		Skull* skulls					{ nullptr };
		Mouns* mounsList				{ nullptr };
		Obstacle* obstacles				{ nullptr };
		MovingPlatform* movingPlatforms	{ nullptr };
		size_t backgroundCount			{ 0 };
		size_t playerCount				{ 0 };
		size_t skullCount				{ 0 };
		size_t mounsCount				{ 0 };
		size_t obstacleCount			{ 0 };
		size_t movingPlatformCount		{ 0 };
	};
}
