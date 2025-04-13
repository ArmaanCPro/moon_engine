#pragma  once
#include "renderer_api.h"

namespace moon
{
    class ortho_camera;
    class shader;

    class MOON_API renderer
    {
    public:
        static void begin_scene(const ortho_camera& cam);
        static void end_scene();

        static void submit(const std::shared_ptr<shader>& shader, const std::shared_ptr<vertex_array>& vertex_array, const glm::mat4& transform = glm::mat4(1.0f));

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
