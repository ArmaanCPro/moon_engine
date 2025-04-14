#pragma once

#include "moon/core/core.h"

#include <cstdint>
#include <string_view>

namespace moon
{
    class MOON_API texture
    {
    public:
        virtual ~texture() = default;

        virtual uint32_t get_width() const = 0;
        virtual uint32_t get_height() const = 0;

        virtual void bind(uint32_t slot = 0) const = 0;
    };

    class texture2d : public texture
    {
    public:
        virtual ~texture2d() override = default;

        static ref<texture2d> create(std::string_view path);
    };
}
