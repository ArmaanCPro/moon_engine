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
        shader* vertex_shader = nullptr;
        shader* fragment_shader = nullptr;
        shader* rootsig_shader = nullptr;
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
