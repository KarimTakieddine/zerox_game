#include "zerox_game.h"
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

        return true;
    }

    void updateGame(GameAllocator* allocator)
    {
        
    }
}