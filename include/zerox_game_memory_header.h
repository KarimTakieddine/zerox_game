#pragma once

#include <cstdint>

namespace ZeroXGame
{
	enum class MemoryType : uint64_t
	{
		UNKNOWN				= 0,
		INPUT				= 1,
		LEVEL				= 2,
		BACKGROUND			= 3,
		OBSTACLE			= 4,
		MOVING_PLATFORM		= 5,
		PLAYER				= 6,
		SKULL				= 7,
		MOUNS				= 8,
		ANIMATION			= 9,
		ANIMATION_FRAME		= 10,
		COLLIDER_ENTITY		= 11,
		RENDER_DESCRIPTOR	= 12,
		ENTITY_SECTION		= 13,
		LAST				= ENTITY_SECTION
	};

	struct alignas(16) GameMemoryHeader
	{
		MemoryType type	{ MemoryType::UNKNOWN };
		uint64_t count	{ 0 };
	};
}
