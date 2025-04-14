#include "moon/events/key_event.h"
#include "moon/events/mouse_event.h"
#include "moon/renderer/texture.h"
#include "opengl/opengl_texture.h"

#include <moon.h>

#include <imgui.h>

#include <opengl/opengl_shader.h>
#include <glm/gtc/type_ptr.hpp>

class sandbox_layer : public moon::layer
{
public:
    sandbox_layer()
        :
        layer("sandbox"),
        camera_controller_(16.0f / 9.0f, true)
    {
        vertex_array_ = moon::ref<moon::vertex_array>(moon::vertex_array::create());

        constexpr float verts[3 * 7] = {
            -0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.7f, 1.0f,
             0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.85f, 1.0f,
             0.0f,  0.5f, 0.0f, 0.7f, 0.8f, 0.2f, 1.0f,
        };
        moon::ref<moon::vertex_buffer> vbuf = moon::ref<moon::vertex_buffer>(moon::vertex_buffer::create(verts, sizeof(verts)));
        moon::buffer_layout layout = {
            { moon::ShaderDataType::Float3, "a_Pos" },
            { moon::ShaderDataType::Float4, "a_Color" }
        };
        vbuf->set_layout(layout);
        vertex_array_->add_vertex_buffer(vbuf);

        constexpr uint32_t indices[3] = { 0, 1, 2 };
        moon::ref<moon::index_buffer> ibuf = moon::ref<moon::index_buffer>(moon::index_buffer::create(indices, sizeof(indices) / sizeof(uint32_t)));
        vertex_array_->set_index_buffer(ibuf);

        // shader
        std::string vertex_shader_src = R"(
            #version 460 core
            layout (location = 0) in vec3 a_Pos;
            layout (location = 1) in vec4 a_Color;

            uniform mat4 u_VP = mat4(1.0);
            uniform mat4 u_Model = mat4(1.0);

            out vec4 v_Color;
            void main()
            {
                v_Color = a_Color;
                gl_Position = u_VP * u_Model * vec4(a_Pos, 1.0);
            }
        )";
        std::string fragment_shader_src = R"(
            #version 460 core
            out vec4 FragColor;
            in vec4 v_Color;
            void main()
            {
                FragColor = v_Color;
            }
        )";

        shader_ = moon::shader::create("vertex pos color", vertex_shader_src, fragment_shader_src);

        square_va_ = moon::ref<moon::vertex_array>(moon::vertex_array::create());
        constexpr float square_verts[5 * 4] = {
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
             0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
             0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
            -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
        };
        uint32_t square_indices[6] = {0, 1, 2, 2, 3, 0};
        moon::ref<moon::vertex_buffer> square_vb = moon::ref<moon::vertex_buffer>(moon::vertex_buffer::create(
            &square_verts[0], sizeof(square_verts)));
        square_vb->set_layout({
            { moon::ShaderDataType::Float3, "a_Pos" },
            { moon::ShaderDataType::Float2, "a_TexCoord" }
        });
        square_va_->add_vertex_buffer(square_vb);

        moon::ref<moon::index_buffer> square_ib = moon::ref<moon::index_buffer>(moon::index_buffer::create(
            &square_indices[0], sizeof(square_indices) / sizeof(uint32_t)));
        square_va_->set_index_buffer(square_ib);

        std::string flat_color_vertex_src = R"(
            #version 460 core
            layout (location = 0) in vec3 a_Pos;

            uniform mat4 u_VP = mat4(1.0);
            uniform mat4 u_Model = mat4(1.0);

            void main()
            {
                gl_Position = u_VP * u_Model * vec4(a_Pos, 1.0);
            }
        )";
        std::string flat_color_fragment_src = R"(
            #version 460 core
            layout(location = 0) out vec4 FragColor;

            uniform vec3 u_Color;

            void main()
            {
                FragColor = vec4(u_Color, 1.0);
            }
        )";

        flat_color_shader_ = moon::shader::create("flat color", flat_color_vertex_src, flat_color_fragment_src);

        auto texture_shader = shader_library_.load("assets/shaders/texture.glsl");

        texture_ = moon::texture2d::create("assets/textures/Checkerboard.png");
        moon_logo_texture_ = moon::texture2d::create("assets/textures/MoonLogo.png");

        std::dynamic_pointer_cast<moon::opengl_shader>(texture_shader)->bind();
        std::dynamic_pointer_cast<moon::opengl_shader>(texture_shader)->upload_uniform_int("u_Texture", 0);
    }

    void on_update(moon::timestep ts) override
    {
        camera_controller_.on_update(ts);

        moon::render_command::set_clear_color({0.1f, 0.1f, 0.1f, 1.0f } );
        moon::render_command::clear();

        moon::renderer::begin_scene(camera_controller_.get_camera());

        static glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

        std::dynamic_pointer_cast<moon::opengl_shader>(flat_color_shader_)->bind();
        std::dynamic_pointer_cast<moon::opengl_shader>(flat_color_shader_)->upload_uniform_float3("u_Color", square_color_);
        for (int y = 0; y < 20; ++y)
        {
            for (int x = 0; x < 20; ++x)
            {
                glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
                glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
                moon::renderer::submit(flat_color_shader_, square_va_, transform);
            }
        }

        auto texture_shader = shader_library_.get("texture");

        texture_->bind(0);
        moon::renderer::submit(texture_shader, square_va_, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));
        moon_logo_texture_->bind(0);
        moon::renderer::submit(texture_shader, square_va_,
            glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

        // Triangle
        //moon::renderer::submit(shader_, vertex_array_);

        moon::renderer::end_scene();
    }

    void on_imgui_render() override
    {
        ImGui::Begin("Settings");
        ImGui::ColorEdit3("Square Color", glm::value_ptr(square_color_));
        ImGui::End();
    }

    void on_event(moon::event& e) override
    {
        camera_controller_.on_event(e);
    }

private:
    moon::shader_library shader_library_;

    moon::ref<moon::vertex_array> vertex_array_;
    moon::ref<moon::shader> shader_;

    moon::ref<moon::vertex_array> square_va_;
    moon::ref<moon::shader> flat_color_shader_;

    moon::ref<moon::texture2d> texture_;
    moon::ref<moon::texture2d> moon_logo_texture_;

    moon::orthographic_camera_controller camera_controller_;

    glm::vec3 square_color_ { 0.2f, 0.3f, 0.4f };
};

class sandbox_app : public moon::application
{
public:
    sandbox_app()
    {
        MOON_INFO("sandbox app created");
        push_layer(new sandbox_layer());
    }

private:

};

moon::application* moon::create_application()
{
    return new sandbox_app();
}
