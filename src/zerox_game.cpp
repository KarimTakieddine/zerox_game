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

    void updateGame(GameAllocator* allocator)
    {
        GameMemory gameMemory = readGameMemory(allocator);
    }
}