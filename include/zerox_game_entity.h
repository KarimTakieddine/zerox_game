#pragma once

#include <glm/vec2.hpp>

#include "zerox_game_transform.h"
#include "zerox_game_material.h"

namespace ZeroXGame
{
    struct alignas(16) PhysicsState
    {
        glm::vec2 lastPosition      { 0.0f, 0.0f };
        glm::vec2 currentPosition   { 0.0f, 0.0f };
    };

    struct alignas(16) GameEntity
    {
        Transform transform;
        Material material;
        PhysicsState physicsState;
    };
}
