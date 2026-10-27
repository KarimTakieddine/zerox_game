#pragma once

#include <glm/matrix.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/vec_swizzle.hpp>

namespace ZeroXGame
{
    struct alignas(16) Transform
    {
        glm::mat4 localToWorld	{ 1.0f };
        glm::mat4 localRotation	{ 1.0f };
        glm::mat4 localScale	{ 1.0f };
    };

    inline glm::vec2 getPosition(const Transform& transform)
    {
        return glm::xy(transform.localToWorld[0]);
    }

    inline void setPosition(Transform& transform, const glm::vec2& position)
    {
        glm::vec4& p = transform.localToWorld[3];

        p.x = position.x;
        p.y = position.y;
    }
}
