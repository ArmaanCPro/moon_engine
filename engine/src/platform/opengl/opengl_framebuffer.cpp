#include "opengl_framebuffer.h"

#include <glad/glad.h>

namespace moon
{
    static const uint32_t s_max_framebuffer_size = 8192;

    opengl_framebuffer::opengl_framebuffer(const framebuffer_spec& spec)
        :
        m_spec_(spec)
    {
        invalidate();
    }

    opengl_framebuffer::~opengl_framebuffer()
    {
        glDeleteFramebuffers(1, &m_renderer_id_);
        glDeleteTextures(1, &m_color_attachment_);
        glDeleteTextures(1, &m_depth_attachment_);
    }

    void opengl_framebuffer::invalidate()
    {
        if (m_renderer_id_)
        {
            glDeleteFramebuffers(1, &m_renderer_id_);
            glDeleteTextures(1, &m_color_attachment_);
            glDeleteTextures(1, &m_depth_attachment_);
        }

        glCreateFramebuffers(1, &m_renderer_id_);
        glBindFramebuffer(GL_FRAMEBUFFER, m_renderer_id_);

        glCreateTextures(GL_TEXTURE_2D, 1, &m_color_attachment_);
        glBindTexture(GL_TEXTURE_2D, m_color_attachment_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)m_spec_.width, (GLsizei)m_spec_.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color_attachment_, 0);

        glCreateTextures(GL_TEXTURE_2D, 1, &m_depth_attachment_);
        glBindTexture(GL_TEXTURE_2D, m_depth_attachment_);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, (GLsizei)m_spec_.width, (GLsizei)m_spec_.height);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_depth_attachment_, 0);

        MOON_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void opengl_framebuffer::bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_renderer_id_);
        glViewport(0, 0, m_spec_.width, m_spec_.height);
    }

    void opengl_framebuffer::unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void opengl_framebuffer::resize(uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0 || width > s_max_framebuffer_size || height > s_max_framebuffer_size)
        {
            MOON_CORE_WARN("Attempted to resize framebuffer to {0}, {1}", width, height);
            return;
        }

        m_spec_.width = width;
        m_spec_.height = height;
        invalidate();
    }
}
