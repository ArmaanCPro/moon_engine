#pragma once

#include "moon/core/core.h"
#include "moon/renderer/texture.h"

namespace moon
{
    struct MOON_API framebuffer_spec
    {
        uint32_t width {}, height {};
        //framebuffer_format format;
        uint32_t samples = 1;

        bool swap_chain_target = false;
    };

    class MOON_API framebuffer
    {
    public:
        virtual ~framebuffer() = default;

        virtual void bind() = 0;
        virtual void unbind() = 0;

        virtual void resize(uint32_t width, uint32_t height) = 0;

        //[[nodiscard]] virtual framebuffer_spec& get_spec() = 0;
        [[nodiscard]] virtual const framebuffer_spec& get_spec() const = 0;
        [[nodiscard]] virtual uint32_t get_color_attachment_renderer_id() const = 0;

        static ref<framebuffer> create(const framebuffer_spec& spec);
    };
}
