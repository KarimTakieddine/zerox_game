#pragma once

#include <memory_view.hpp>
#include <memory_cursor.hpp>
#include <zerox_input.h>

#include "zerox_enemy_types.h"
#include "zerox_game_animation.h"
#include "zerox_game_background.h"
#include "zerox_game_collider.h"
#include "zerox_game_constants.h"
#include "zerox_game_player.h"
#include "zerox_game_level.h"
#include "zerox_obstacle_types.h"

namespace ZeroXGame
{
    struct GameMemory
    {
        std::span<ZeroXBridge::Input> input;
        std::span<Level> levels;
        std::span<Animation> animations;
        std::span<Background> backgrounds;
        std::span<Player> players;
        std::span<Skull> skulls;
        std::span<Mouns> mounsSpan;
        std::span<Obstacle> obstacles;
        std::span<MovingPlatform> movingPlatforms;
        std::span<Collider2DEntity> collider2DEntities;
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
            cursor.step_array<Skull>(level->skullCount);
            cursor.step_array<Mouns>(level->mounsCount);
            cursor.step_array<Obstacle>(level->obstacleCount);
            cursor.step_array<MovingPlatform>(level->movingPlatformCount);
        }

        const auto animations = memory.animations;
        cursor.step_array<Animation>(animations.size());

        for (size_t i = 0; i < animations.size(); ++i)
        {
            cursor.step_array<AnimationFrame>((animations.data() + i)->frameCount);
        }

        return cursor.getOffset();
    }

    /*inline GameMemory readGameMemory(const GameAllocator* allocator)
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
            gameMemoryCursor.step_array<Skull>(level->skullCount);
            gameMemoryCursor.step_array<Mouns>(level->mounsCount);
            gameMemoryCursor.step_array<Obstacle>(level->obstacleCount);
            gameMemoryCursor.step_array<MovingPlatform>(level->movingPlatformCount);
        }

        result.animations = gameMemoryView.read_contiguous_array<Animation>(gameMemoryCursor.getOffset());
        gameMemoryCursor.step_array<Animation>(result.animations.size());

        for (size_t i = 0; i < result.animations.size(); ++i)
        {
            gameMemoryCursor.step_array<AnimationFrame>((result.animations.data() + i)->frameCount);
        }

        if (gameMemoryCursor.getOffset() >= gameMemorySpan.size())
        {
            return result;
        }

        result.backgrounds = gameMemoryView.read_contiguous_array<Background>(gameMemoryCursor.getOffset());
        gameMemoryCursor.step_array<Background>(result.backgrounds.size());
        
        result.players = gameMemoryView.read_contiguous_array<Player>(gameMemoryCursor.getOffset());
        gameMemoryCursor.step_array<Player>(result.players.size());

        result.skulls = gameMemoryView.read_contiguous_array<Skull>(gameMemoryCursor.getOffset());
        gameMemoryCursor.step_array<Skull>(result.skulls.size());

        result.mounsSpan = gameMemoryView.read_contiguous_array<Mouns>(gameMemoryCursor.getOffset());
        gameMemoryCursor.step_array<Mouns>(result.mounsSpan.size());

        result.obstacles = gameMemoryView.read_contiguous_array<Obstacle>(gameMemoryCursor.getOffset());
        gameMemoryCursor.step_array<Obstacle>(result.obstacles.size());

        result.movingPlatforms = gameMemoryView.read_contiguous_array<MovingPlatform>(gameMemoryCursor.getOffset());
        gameMemoryCursor.step_array<MovingPlatform>(result.movingPlatforms.size());

        result.collider2DEntities = gameMemoryView.read_contiguous_array<Collider2DEntity>(gameMemoryCursor.getOffset());
        gameMemoryCursor.step_array<Collider2DEntity>(result.collider2DEntities.size());

        result.entitySectionCount = gameMemoryView.read_object<uint64_t>(gameMemoryCursor.getOffset());
        gameMemoryCursor.step<uint64_t>();

        result.entitySectionSpan = gameMemorySpan.subspan(
            gameMemoryCursor.getOffset(),
            allocator->getLiveSizeInBytes() - gameMemoryCursor.getOffset());
        
        return result;
    }*/
}
