#pragma once

#include <memory_view.hpp>
#include <memory_cursor.hpp>
#include <zerox_input.h>

#include "zerox_game.h"
#include "zerox_game_background.h"
#include "zerox_game_collider.h"
#include "zerox_game_enemy.h"
#include "zerox_game_obstacle.h"
#include "zerox_game_player.h"

namespace ZeroXGame
{
    struct GameMemory
    {
        std::span<ZeroXBridge::Input> input;
        std::span<Background> backgrounds;
        std::span<Player> players;
        std::span<Enemy> enemies;
        std::span<Obstacle> obstacles;
        std::span<Collider> colliders;
        std::span<uint64_t> entitySectionCount;
        std::span<std::byte> entitySectionSpan;
    };

    inline GameMemory readGameMemory(const GameAllocator* allocator)
    {
        GameMemory result;

        std::span<std::byte> gameMemorySpan(reinterpret_cast<std::byte*>(allocator->peek()), allocator->getLiveSizeInBytes());
        MutableMemoryView gameMemoryView(gameMemorySpan);

        MemoryCursor<GAME_MEMORY_ALIGNMENT> gameMemoryCursor;

        result.input = gameMemoryView.read_object<ZeroXBridge::Input>(gameMemoryCursor.getOffset());
        gameMemoryCursor.step<ZeroXBridge::Input>();

        result.backgrounds = gameMemoryView.read_contiguous_array<Background>(gameMemoryCursor.getOffset());
        gameMemoryCursor.step_array<Background>(result.backgrounds.size());
        
        result.players = gameMemoryView.read_contiguous_array<Player>(gameMemoryCursor.getOffset());
        gameMemoryCursor.step_array<Player>(result.players.size());

        result.enemies = gameMemoryView.read_contiguous_array<Enemy>(gameMemoryCursor.getOffset());
        gameMemoryCursor.step_array<Enemy>(result.enemies.size());

        result.obstacles = gameMemoryView.read_contiguous_array<Obstacle>(gameMemoryCursor.getOffset());
        gameMemoryCursor.step_array<Obstacle>(result.obstacles.size());

        result.colliders = gameMemoryView.read_contiguous_array<Collider>(gameMemoryCursor.getOffset());
        gameMemoryCursor.step_array<Collider>(result.colliders.size());

        result.entitySectionCount = gameMemoryView.read_object<uint64_t>(gameMemoryCursor.getOffset());
        gameMemoryCursor.step<uint64_t>();

        result.entitySectionSpan = gameMemorySpan.subspan(
            gameMemoryCursor.getOffset(),
            allocator->getLiveSizeInBytes() - gameMemoryCursor.getOffset());
        
        return result;
    }
}
