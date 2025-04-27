#include "moonpch.h"

#include "renderer_api.h"

namespace moon
{
#ifdef _WIN32
    renderer_api::API renderer_api::s_API_ = renderer_api::API::DirectX;
#else
    renderer_api::API renderer_api::s_API_ = renderer_api::API::OpenGL;
#endif
}
