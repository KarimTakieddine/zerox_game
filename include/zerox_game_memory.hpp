#pragma once

#include <memory_view.hpp>
#include <memory_cursor.hpp>

#include "zerox_game.h"

namespace ZeroXGame
{
    struct GameMemory
    {
        std::span<GameEntity> gameEntities;
    };

    inline GameMemory readGameMemory(GameAllocator* allocator)
    {
        GameMemory result;

        std::span<std::byte> gameMemorySpan(reinterpret_cast<std::byte*>(allocator->peek()), allocator->getLiveSizeInBytes());
        MutableMemoryView gameMemoryView(gameMemorySpan);

        MemoryCursor<GAME_MEMORY_ALIGNMENT> gameMemoryCursor;
        
        result.gameEntities = gameMemoryView.read_contiguous_array<GameEntity>(gameMemoryCursor.getOffset());

        gameMemoryCursor.step_array<GameEntity>(result.gameEntities.size());
        
        return result;
    }
}
