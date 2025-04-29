#pragma once

#include "shader.h"
#include "texture.h"
#include "vertex_array.h"
#include "moon/core/core.h"

#include <vector>

namespace moon
{
    struct pipeline_spec
    {
        // TODO: shaders already hold a ShaderType, maybe reconsider this in future?
        std::unordered_map<ShaderType, shader*> shaders;
        std::vector<texture*> textures;

        // TODO: consider removing vertex arrays entirely
        vertex_array* vertex_array;

        bool enable_depth_testing = false;
        bool enable_blending = false;
    };

    class MOON_API pipeline
    {
    public:
        virtual ~pipeline() = default;

        virtual void bind() = 0;
        virtual void unbind() = 0;

        static ref<pipeline> create(const pipeline_spec& spec);

    };
}
