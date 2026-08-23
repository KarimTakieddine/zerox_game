#include "zerox_game.h"
#include "zerox_game_entity.h"
#include "zerox_game_memory.hpp"

#include <iostream>

namespace ZeroXGame
{
    bool initializeGame(GameAllocator* allocator)
    {
        if (!allocator)
        {
            std::cout << "No allocator supplied to game library" << std::endl;
            return false;
        }

        std::cout << "Game library successfully loaded. Initializing game memory..." << std::endl;

        const size_t playerCount{ 1 };
        allocator->requestMemory<uint64_t>(playerCount);
        allocator->requestMemoryArray<GameEntity>(playerCount);

        const size_t enemyCount{ 4 };
        allocator->requestMemory<uint64_t>(enemyCount);
        allocator->requestMemoryArray<GameEntity>(enemyCount);

        return true;
    }

    void updateGame(GameAllocator* allocator, double deltaTime)
    {
        std::cout << deltaTime << std::endl;

        GameMemory gameMemory = readGameMemory(allocator);
        
        const auto players = gameMemory.players;
        for (size_t i = 0; i < players.size(); ++i)
        {
            auto* player = players.data() + i;

            player->transform.localToWorld[3].x -= 0.35f * deltaTime;
        }

        const auto enemies = gameMemory.enemies;
        for (size_t i = 0; i < enemies.size(); ++i)
        {
            auto* enemy = enemies.data() + i;

            enemy->transform.localToWorld[3].x += (i + 1) * 0.25f * deltaTime;
        }
    }
}