#pragma  once

#include <cstdint>

#include <glm/glm.hpp>

namespace moon
{
    class ortho_camera;
    class shader;
    class vertex_buffer;
    class index_buffer;

    struct renderer_api
    {
        enum class API
        {
            None = 0,
            Vulkan = 1,
        };

        API api;
    };

    class MOON_API renderer
    {
    public:
        static void init();
        static void shutdown();
        static void on_window_resize(uint32_t width, uint32_t height);

        static void begin_scene(const ortho_camera& cam);
        static void end_scene();

        static renderer_api::API get_api() { return s_api_.api; };

        static void submit(const ref<shader>& shader, const ref<vertex_buffer>& vertex_array, const ref<index_buffer>& index_buffer, const glm::mat4& transform = glm::mat4(1.0f));

        inline static const glm::mat4& get_view_projection_matrix() { return s_scene_data_->view_projection_matrix; }

    private:
        struct scene_data
        {
            glm::mat4 view_projection_matrix;
        };

        static scene_data* s_scene_data_;

        static renderer_api s_api_;
    };
}
