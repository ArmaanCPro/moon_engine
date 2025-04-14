#pragma once

#include "../../moon/core.h"

namespace moon
{
    class MOON_API graphics_context
    {
    public:
        virtual ~graphics_context() = default;

        virtual void init() = 0;
        virtual void swap_buffers() = 0;
    };
}
