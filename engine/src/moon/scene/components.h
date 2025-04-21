#pragma once

#include <glm/glm.hpp>

namespace moon
{
    struct MOON_API transform_component
    {
        glm::mat4 transform = glm::mat4{ 1.0f };

        transform_component() = default;
        transform_component(const transform_component&) = default;
        explicit transform_component(const glm::mat4& transform)
        : transform(transform) {}

        operator glm::mat4&() { return transform; }
        operator const glm::mat4&() const { return transform; }
    };

    struct MOON_API sprite_renderer_component
    {
        glm::vec4 color = glm::vec4{ 1.0f };

        sprite_renderer_component() = default;
        sprite_renderer_component(const sprite_renderer_component& other) = default;
        explicit sprite_renderer_component(const glm::vec4& color)
        : color(color) {}
    };
}
