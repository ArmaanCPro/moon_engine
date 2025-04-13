#pragma  once
#include "renderer_api.h"

namespace moon
{
    class camera;

    class MOON_API renderer
    {
    public:
        static void begin_scene(const camera& cam);
        static void end_scene();

        static void submit(const std::shared_ptr<vertex_array>& vertex_array);

        inline static renderer_api::API get_api() { return renderer_api::get_api(); }
        inline static const glm::mat4& get_view_projection_matrix() { return s_vp_matrix_; }

    private:
        static glm::mat4 s_vp_matrix_;
    };
}
