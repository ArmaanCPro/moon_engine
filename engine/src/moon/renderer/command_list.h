#pragma once

#include "moon/core/core.h"
#include "renderer_api.h"

namespace moon
{
    class MOON_API command_list
    {
    public:
        virtual ~command_list() = default;

        virtual void reset() = 0;
        virtual void begin() = 0;
        virtual void end() = 0;
        virtual void submit() = 0;

        virtual void draw_indexed(const ref<vertex_array>& vertex_array, uint32_t index_count = 0) = 0;
        virtual void dispatch() = 0;

        // TEMP
        virtual void* get_native_handle() = 0;
    };
}
