#include "moonpch.h"
#include "render_command.h"

#include "platform/d3d12/d3d12_renderer_api.h"
#include "platform/opengl/opengl_renderer_api.h"

namespace moon
{
    // TODO fix code that doesn't adhere to our renderer api
#ifdef _WIN32
    renderer_api* render_command::s_renderer_api_ = new d3d12_renderer_api();
#else
    renderer_api* render_command::s_renderer_api_ = new opengl_renderer_api();
#endif
}
