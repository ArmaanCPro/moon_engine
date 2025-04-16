#pragma  once

#include "renderer_api.h"

#include <cstdint>

namespace moon
{
    class ortho_camera;
    class shader;

    class MOON_API renderer
    {
    public:
        static void init();
        static void shutdown();
        static void on_window_resize(uint32_t width, uint32_t height);

        static void begin_scene(const ortho_camera& cam);
        static void end_scene();

        static void submit(const ref<shader>& shader, const ref<vertex_array>& vertex_array, const glm::mat4& transform = glm::mat4(1.0f));

        inline static renderer_api::API get_api() { return renderer_api::get_api(); }
        inline static const glm::mat4& get_view_projection_matrix() { return s_scene_data_->view_projection_matrix; }

    private:
        struct scene_data
        {
            glm::mat4 view_projection_matrix;
        };

        static scene_data* s_scene_data_;
    };
}
