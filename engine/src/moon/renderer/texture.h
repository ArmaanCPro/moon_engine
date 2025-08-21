#pragma once

#include "handle.h"
#include "moon/core/core.h"

#include <cstdint>
#include <string_view>

namespace moon
{
    class MOON_API texture
    {
    public:
        virtual ~texture() = default;

        [[nodiscard]] virtual uint32_t get_width() const = 0;
        [[nodiscard]] virtual uint32_t get_height() const = 0;
        [[nodiscard]] virtual uint32_t get_renderer_id() const = 0;

        virtual void set_data(void* data, uint32_t size) = 0;

        virtual void bind(uint32_t slot = 0) const = 0;

        virtual texture_handle get_handle() = 0;

        virtual bool operator==(const texture& other) const = 0;
    };

    class MOON_API texture2d : public texture
    {
    public:
        virtual ~texture2d() override = default;

        static ref<texture2d> create(uint32_t width, uint32_t height);
        static ref<texture2d> create(std::string_view path);
    };
}
