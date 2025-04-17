#pragma once

#include "moon/renderer/texture.h"
#include <string_view>
#include <glad/glad.h>

namespace moon
{
    class opengl_texture2d : public texture2d
    {
    public:
        opengl_texture2d(uint32_t width, uint32_t height);
        opengl_texture2d(std::string_view path);
        ~opengl_texture2d() override;

        uint32_t get_width() const override { return width_; }
        uint32_t get_height() const override { return height_; }

        void set_data(void* data, uint32_t size) override;

        void bind(uint32_t slot = 0) const override;

        bool operator==(const texture& other) const override
        {
            return renderer_id_ == ((opengl_texture2d&)other).renderer_id_;
        }

    private:
        std::string path_;
        uint32_t width_, height_;
        uint32_t renderer_id_;

        GLenum internal_format_, data_format_;
    };
}
