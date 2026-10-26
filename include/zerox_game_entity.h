#pragma once

#include "zerox_game_transform.h"
#include "zerox_game_material.h"

namespace ZeroXGame
{
    struct alignas(16) GameEntity
    {
        Transform transform;
        Material material;
    };
}
