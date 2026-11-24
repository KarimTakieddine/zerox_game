#pragma once

#include <bump_allocator.hpp>

namespace ZeroXGame
{
	inline constexpr size_t GAME_MEMORY_ALIGNMENT = 16;
	
	using GameAllocator = BumpAllocator<GAME_MEMORY_ALIGNMENT>;
}
