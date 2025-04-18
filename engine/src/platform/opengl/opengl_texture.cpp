#include "moonpch.h"
#include "opengl_texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glad/glad.h>

#include <cmrc/cmrc.hpp>

CMRC_DECLARE(textures);

namespace moon
{
    opengl_texture2d::opengl_texture2d(uint32_t width, uint32_t height)
        :
        width_(width), height_(height)
    {
        MOON_PROFILE_FUNCTION();

        internal_format_ = GL_RGBA8;
        data_format_ = GL_RGBA;

        glCreateTextures(GL_TEXTURE_2D, 1, &renderer_id_);
        glTextureStorage2D(renderer_id_, 1, internal_format_, width_, height_);

        glTextureParameteri(renderer_id_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(renderer_id_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTextureParameteri(renderer_id_, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(renderer_id_, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    opengl_texture2d::opengl_texture2d(std::string_view path)
        :
        path_(path.data())
    {
        MOON_PROFILE_FUNCTION();

        stbi_set_flip_vertically_on_load(true);
        int width, height, channels;
        stbi_uc* data = nullptr;

        {
            MOON_PROFILE_SCOPE("stbi_load - opengl_texture2d::opengl_texture2d(std::string_view path)");

            // Try loading from embedded resources first
            try
            {
                auto fs = cmrc::textures::get_filesystem();
                if (fs.exists(path.data()))
                {
                    auto resource = fs.open(path.data());
                    data = stbi_load_from_memory(
                        (const stbi_uc*)resource.begin(),
                        (int)resource.size(),
                        &width, &height, &channels, 0
                        );
                    MOON_CORE_INFO("Loading embedded texture: {0}", path.data());
                }
                else
                {
                    // Fallback to filesystem loading
                    data = stbi_load(path.data(), &width, &height, &channels, 0);
                    MOON_CORE_INFO("Loading file system texture: {0}", path.data());
                }
            }
            catch (const std::exception& e)
            {
                // Fallback to filesystem loading
                MOON_CORE_WARN("Failed to load from embedded resources: {0}, trying filesystem", e.what());
                data = stbi_load(path.data(), &width, &height, &channels, 0);
            }
        }
        MOON_CORE_ASSERT(data, "Failed to load image!");

        width_ = width;
        height_ = height;

        GLenum internal_format = 0, data_format = 0;
        if (channels == 4)
        {
            internal_format = GL_RGBA8;
            data_format = GL_RGBA;
        }
        else if (channels == 3)
        {
            internal_format = GL_RGB8;
            data_format = GL_RGB;
        }

        internal_format = internal_format ? internal_format : GL_RGBA;
        data_format = data_format ? data_format : GL_RGBA;

        MOON_CORE_ASSERT(internal_format & data_format, "Format not supported!");

        glCreateTextures(GL_TEXTURE_2D, 1, &renderer_id_);
        glTextureStorage2D(renderer_id_, 1, internal_format, width_, height_);

        glTextureParameteri(renderer_id_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(renderer_id_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTextureParameteri(renderer_id_, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(renderer_id_, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTextureSubImage2D(renderer_id_, 0, 0, 0, width_, height_, data_format, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
    }

    opengl_texture2d::~opengl_texture2d()
    {
        MOON_PROFILE_FUNCTION();

        glDeleteTextures(1, &renderer_id_);
    }

    void opengl_texture2d::set_data(void* data, uint32_t size)
    {
        MOON_PROFILE_FUNCTION();

        uint32_t bpp = data_format_ == GL_RGBA ? 4 : 3;
        MOON_CORE_ASSERT(size == width_ * height_ * bpp, "Data must be entire texture!");
        glTextureSubImage2D(renderer_id_, 0, 0, 0, width_, height_, data_format_, GL_UNSIGNED_BYTE, data);
    }

    void opengl_texture2d::bind(uint32_t slot) const
    {
        MOON_PROFILE_FUNCTION();

        glBindTextureUnit(slot, renderer_id_);
    }
}
