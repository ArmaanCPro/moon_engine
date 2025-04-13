#include "moonpch.h"
#include "render_command.h"

#include "opengl/opengl_renderer_api.h"

namespace moon
{
    renderer_api* render_command::s_renderer_api_ = new opengl_renderer_api();
}
