#pragma once

#include <memory_view.hpp>
#include <memory_cursor.hpp>

#include "zerox_game.h"

namespace ZeroXGame
{
    struct GameMemory
    {
        std::span<GameEntity> players;
        std::span<GameEntity> enemies;
        std::span<GameEntity> obstacles;
        std::span<GameEntity> backgrounds;
    };

    inline GameMemory readGameMemory(GameAllocator* allocator)
    {
        GameMemory result;

        std::span<std::byte> gameMemorySpan(reinterpret_cast<std::byte*>(allocator->peek()), allocator->getLiveSizeInBytes());
        MutableMemoryView gameMemoryView(gameMemorySpan);

        MemoryCursor<GAME_MEMORY_ALIGNMENT> gameMemoryCursor;
        
        result.players = gameMemoryView.read_contiguous_array<GameEntity>(gameMemoryCursor.getOffset());

        gameMemoryCursor.step_array<GameEntity>(result.players.size());
        
        return result;
    }
}
