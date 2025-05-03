#pragma once

#include "command_list.h"
#include "moon/core/core.h"

namespace moon
{
    class MOON_API graphics_context
    {
    public:
        virtual ~graphics_context() = default;

        virtual void init() = 0;
        virtual void swap_buffers() = 0;

        virtual void begin_frame() {};
        virtual void end_frame() {};

        virtual void flush([[maybe_unused]] size_t count) {};

        virtual command_list* get_command_list(uint32_t frame_index) = 0;

        static graphics_context* create(void* window_handle);
    };
}
