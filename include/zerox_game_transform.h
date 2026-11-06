#pragma once

#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/vec_swizzle.hpp>

#include <zerox_transform.h>

namespace ZeroXGame
{
    inline glm::vec2 getPosition(const ZeroXBridge::Transform& transform)
    {
        return glm::xy(transform.localToWorld[3]);
    }

    inline glm::vec2 getScale(const ZeroXBridge::Transform& transform)
    {
        const glm::mat4& localScale = transform.localScale;

        return { localScale[0].x, localScale[1].y };
    }

    inline void setPosition(ZeroXBridge::Transform& transform, const glm::vec2& position)
    {
        glm::vec4& p = transform.localToWorld[3];

        p.x = position.x;
        p.y = position.y;
    }

    inline void setLocalScale(ZeroXBridge::Transform& transform, const glm::vec2& localScale)
    {
        transform.localScale = glm::scale(glm::mat4{ 1.0f }, glm::vec3(localScale, 1.0f));
    }
}
