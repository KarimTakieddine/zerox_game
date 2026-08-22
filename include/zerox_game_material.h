#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace ZeroXGame
{
    struct Material
    {
        glm::vec4 color         { 1.0f, 1.0f, 1.0f, 1.0f };
        glm::vec2 textureOffset { 0.0f, 0.0f };
        glm::vec2 uvScale       { 0.0f, 0.0f };
    };
}
