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
        std::span<uint64_t> entitySectionCount;
        std::span<std::byte> entitySectionSpan;
    };

    inline GameMemory readGameMemory(const GameAllocator* allocator)
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

        result.entitySectionCount = gameMemoryView.read_object<uint64_t>(gameMemoryCursor.getOffset());

        gameMemoryCursor.step<uint64_t>();

        const size_t sectionSpanOffset  = gameMemoryCursor.getOffset();
        result.entitySectionSpan        = gameMemorySpan.subspan(
            sectionSpanOffset,
            gameMemorySpan.size() - sectionSpanOffset);
        
        return result;
    }
}
