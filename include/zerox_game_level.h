#pragma once

namespace ZeroXGame
{
	struct Background;
	struct Player;
	struct Enemy;
	struct Obstacle;

	struct alignas(16) Level
	{
		Background* backgrounds	{ nullptr };
		Player* players			{ nullptr };
		Enemy* enemies			{ nullptr };
		Obstacle* obstacles		{ nullptr };
		size_t backgroundCount	{ 0 };
		size_t playerCount		{ 0 };
		size_t enemyCount		{ 0 };
		size_t obstacleCount	{ 0 };
	};
}
