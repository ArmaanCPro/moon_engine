#pragma  once
#include "renderer_api.h"

namespace moon
{
    class MOON_API renderer
    {
    public:
        static void begin_scene();
        static void end_scene();

        static void submit(const std::shared_ptr<vertex_array>& vertex_array);

        inline static renderer_api::API get_api() { return renderer_api::get_api(); }
    };
}
