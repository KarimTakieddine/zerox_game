#pragma once

#include <cstddef>

#include <bump_allocator.hpp>

#include "zerox_game_api.h"

namespace ZeroXGame
{
    constexpr size_t GAME_MEMORY_ALIGNMENT = 16;

    using GameAllocator = BumpAllocator<GAME_MEMORY_ALIGNMENT>;

    extern "C" ZEROX_GAME_API bool initializeGame(GameAllocator* allocator);
	extern "C" ZEROX_GAME_API void updateGame(GameAllocator* allocator);
}
