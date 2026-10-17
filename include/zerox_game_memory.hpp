#pragma once

#include <memory_view.hpp>
#include <memory_cursor.hpp>
#include <zerox_engine_input.h>

#include "zerox_game.h"
#include "zerox_game_entity.h"

namespace ZeroXGame
{
    struct GameMemory
    {
        std::span<ZeroX::Input> input;
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

        result.input = gameMemoryView.read_object<ZeroX::Input>(gameMemoryCursor.getOffset());

        gameMemoryCursor.step<ZeroX::Input>();
        
        result.players = gameMemoryView.read_contiguous_array<GameEntity>(gameMemoryCursor.getOffset());

        gameMemoryCursor.step_array<GameEntity>(result.players.size());

        result.enemies = gameMemoryView.read_contiguous_array<GameEntity>(gameMemoryCursor.getOffset());

        gameMemoryCursor.step_array<GameEntity>(result.enemies.size());
        
        return result;
    }
}
