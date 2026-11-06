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
#include "zerox_game_level.h"

namespace ZeroXGame
{
    struct GameMemory
    {
        std::span<ZeroXBridge::Input> input;
        std::span<Level> levels;
        std::span<Background> backgrounds;
        std::span<Player> players;
        std::span<Enemy> enemies;
        std::span<Obstacle> obstacles;
        std::span<Collider> colliders;
        std::span<uint64_t> entitySectionCount;
        std::span<std::byte> entitySectionSpan;
    };

    inline size_t getReservedSize(const GameMemory& memory)
    {
        MemoryCursor<GAME_MEMORY_ALIGNMENT> cursor;

        cursor.step<ZeroXBridge::Input>();

        const auto levels = memory.levels;
        cursor.step_array<Level>(levels.size());

        for (size_t i = 0; i < levels.size(); ++i)
        {
            const Level* level = levels.data() + i;

            cursor.step_array<Background>(level->backgroundCount);
            cursor.step_array<Player>(level->playerCount);
            cursor.step_array<Enemy>(level->enemyCount);
            cursor.step_array<Obstacle>(level->obstacleCount);
        }

        return cursor.getOffset();
    }

    inline GameMemory readGameMemory(const GameAllocator* allocator)
    {
        GameMemory result;

        std::span<std::byte> gameMemorySpan(reinterpret_cast<std::byte*>(allocator->peek()), allocator->getLiveSizeInBytes());
        MutableMemoryView gameMemoryView(gameMemorySpan);

        MemoryCursor<GAME_MEMORY_ALIGNMENT> gameMemoryCursor;

        result.input = gameMemoryView.read_object<ZeroXBridge::Input>(gameMemoryCursor.getOffset());
        gameMemoryCursor.step<ZeroXBridge::Input>();

        result.levels = gameMemoryView.read_contiguous_array<Level>(gameMemoryCursor.getOffset());
        gameMemoryCursor.step_array<Level>(result.levels.size());

        for (size_t i = 0; i < result.levels.size(); ++i)
        {
            const auto* level = result.levels.data() + i;

            gameMemoryCursor.step_array<Background>(level->backgroundCount);
            gameMemoryCursor.step_array<Player>(level->playerCount);
            gameMemoryCursor.step_array<Enemy>(level->enemyCount);
            gameMemoryCursor.step_array<Obstacle>(level->obstacleCount);
        }

        if (gameMemoryCursor.getOffset() >= gameMemorySpan.size())
        {
            return result;
        }

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
