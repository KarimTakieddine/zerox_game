#pragma once

#include <cstddef>

#include <zerox_game_context.h>
#include <zerox_render_descriptor.h>

#include "zerox_game_api.h"
#include "zerox_game_constants.h"

namespace ZeroXBridge { struct EngineFunctions; }

namespace ZeroXGame
{
    using ZeroXBridge::RenderDescriptor;

    extern "C" ZEROX_GAME_API void setEngineFunctions(const ZeroXBridge::EngineFunctions* engineFunctions);
    extern "C" ZEROX_GAME_API ZeroXBridge::GameContext initializeGame(GameAllocator* allocator, const ZeroXBridge::EngineFunctions* engineFunctions);
    extern "C" ZEROX_GAME_API bool loadLevel(const ZeroXBridge::GameContext* context, GameAllocator* allocator, size_t index);
	extern "C" ZEROX_GAME_API void updateGame(GameAllocator* allocator, double deltaTime);
    extern "C" ZEROX_GAME_API RenderDescriptor initializeRenderDescriptor(const GameAllocator* allocator);
}
