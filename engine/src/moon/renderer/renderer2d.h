#pragma once

#include "moon/core/core.h"

#include "moon/renderer/camera.h"

namespace moon
{
    class MOON_API renderer2d
    {
    public:
        // system
        static void init();
        static void shutdown();

        static void begin_scene(const ortho_camera& camera);
        static void end_scene();

        // primitives
        static void draw_quad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
        static void draw_quad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
    };
}
