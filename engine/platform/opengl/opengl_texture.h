#pragma once

#include "moon/renderer/texture.h"
#include <string_view>

namespace moon
{
    class opengl_texture2d : public texture2d
    {
    public:
        opengl_texture2d(std::string_view path);
        ~opengl_texture2d() override;

        uint32_t get_width() const override { return width_; }
        uint32_t get_height() const override { return height_; }

        void bind(uint32_t slot = 0) const override;

    private:
        std::string path_;
        uint32_t width_, height_;
        uint32_t renderer_id_;
    };
}
