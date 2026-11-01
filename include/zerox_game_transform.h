#pragma once

#include <glm/matrix.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/vec_swizzle.hpp>

#include <zerox_transform.h>

namespace ZeroXGame
{
    inline glm::vec2 getPosition(const ZeroXBridge::Transform& transform)
    {
        return glm::xy(transform.localToWorld[0]);
    }

    inline void setPosition(ZeroXBridge::Transform& transform, const glm::vec2& position)
    {
        glm::vec4& p = transform.localToWorld[3];

        p.x = position.x;
        p.y = position.y;
    }
}
