#include "moonpch.h"
#include "render_command.h"

#include "core/application.h"
#include "platform/opengl/opengl_renderer_api.h"
#include "vulkan/vk_context.h"
#include "vulkan/vk_renderer_api.h"

namespace moon
{
    renderer_api* render_command::s_renderer_api_ = new vulkan::vk_renderer_api();
}
