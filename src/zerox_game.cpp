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

        allocator->requestMemory<uint64_t>(4);
        for (size_t i = 0; i < 4; ++i)
        {
            auto* gameEntity = allocator->requestMemory<GameEntity>();
            if (!gameEntity)
            {
                std::cout << "Failed to create game entity: " << i << std::endl;
            }
        }

        return true;
    }

    void updateGame(GameAllocator* allocator)
    {
        GameMemory gameMemory = readGameMemory(allocator);
    }
}