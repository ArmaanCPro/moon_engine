#pragma once

#include "moon/core/core.h"

#include "moon/renderer/camera.h"
#include "moon/renderer/texture.h"
#include "moon/renderer/subtexture2d.h"

namespace moon
{
    class MOON_API renderer2d
    {
    public:
        // system
        static void init();
        static void shutdown();

        static void begin_scene(const camera& camera, const glm::mat4& transform);
        static void begin_scene(const ortho_camera& camera); // todo: remove
        static void end_scene();
        static void flush();

        // primitives
        static void draw_quad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
        static void draw_quad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
        static void draw_quad(const glm::vec2& position, const glm::vec2& size, const ref<texture2d>& texture, float tiling_factor = 1.0f, const glm::vec4& tint_color = glm::vec4(1.0f));
        static void draw_quad(const glm::vec3& position, const glm::vec2& size, const ref<texture2d>& texture, float tiling_factor = 1.0f, const glm::vec4& tint_color = glm::vec4(1.0f));
        static void draw_quad(const glm::vec2& position, const glm::vec2& size, const ref<subtexture2d>& subtexture, float tiling_factor = 1.0f, const glm::vec4& tint_color = glm::vec4(1.0f));
        static void draw_quad(const glm::vec3& position, const glm::vec2& size, const ref<subtexture2d>& subtexture, float tiling_factor = 1.0f, const glm::vec4& tint_color = glm::vec4(1.0f));

        static void draw_quad(const glm::mat4& transform, const glm::vec4& color);
        static void draw_quad(const glm::mat4& transform, const ref<texture2d>& texture, float tiling_factor = 1.0f, const glm::vec4& tint_color = glm::vec4(1.0f));

        /// Rotation should be passed in as radians
        static void draw_rotated_quad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color);
        static void draw_rotated_quad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color);
        static void draw_rotated_quad(const glm::vec2& position, const glm::vec2& size, float rotation, const ref<texture2d>& texture, float tiling_factor = 1.0f, const glm::vec4& tint_color = glm::vec4(1.0f));
        static void draw_rotated_quad(const glm::vec3& position, const glm::vec2& size, float rotation, const ref<texture2d>& texture, float tiling_factor = 1.0f, const glm::vec4& tint_color = glm::vec4(1.0f));
        static void draw_rotated_quad(const glm::vec2& position, const glm::vec2& size, float rotation, const ref<subtexture2d>& subtexture, float tiling_factor = 1.0f, const glm::vec4& tint_color = glm::vec4(1.0f));
        static void draw_rotated_quad(const glm::vec3& position, const glm::vec2& size, float rotation, const ref<subtexture2d>& subtexture, float tiling_factor = 1.0f, const glm::vec4& tint_color = glm::vec4(1.0f));

        struct statistics
        {
            uint32_t draw_calls = 0;
            uint32_t quad_count = 0;

            uint32_t get_total_vertex_count() const { return quad_count * 4; }
            uint32_t get_total_index_count() const { return quad_count * 6; }
        };
        static statistics get_stats();
        static void reset_stats();

    private:
        static void flush_and_reset();
    };
}
