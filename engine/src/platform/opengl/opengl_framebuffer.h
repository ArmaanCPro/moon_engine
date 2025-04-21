#pragma once

#include "moon/renderer/framebuffer.h"

namespace moon
{
    class opengl_framebuffer : public framebuffer
    {
    public:
        explicit opengl_framebuffer(const framebuffer_spec& spec);
        ~opengl_framebuffer() override;

        void invalidate();

        void bind() override;
        void unbind() override;
        void resize(uint32_t width, uint32_t height) override;

        uint32_t get_color_attachment_renderer_id() const override { return m_color_attachment_; }
        const framebuffer_spec& get_spec() const override { return m_spec_; }
    private:
        uint32_t m_renderer_id_ {0};
        uint32_t m_color_attachment_ {0}, m_depth_attachment_ {0};
        framebuffer_spec m_spec_;
    };
}
