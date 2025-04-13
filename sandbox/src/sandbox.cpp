#include "core/events/key_event.h"
#include "core/events/mouse_event.h"

#include <moon.h>

#include <imgui.h>

class sandbox_layer : public moon::layer
{
public:
    sandbox_layer()
        : layer("sandbox")
    {
        camera_.translate({ 0.0f, 0.0f, 10.0f });
        vertex_array_ = std::shared_ptr<moon::vertex_array>(moon::vertex_array::create());

        constexpr float verts[3 * 7] = {
            -0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.7f, 1.0f,
             0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.85f, 1.0f,
             0.0f,  0.5f, 0.0f, 0.7f, 0.8f, 0.2f, 1.0f,
        };
        std::shared_ptr<moon::vertex_buffer> vbuf = std::shared_ptr<moon::vertex_buffer>(moon::vertex_buffer::create(verts, sizeof(verts)));
        moon::buffer_layout layout = {
            { moon::ShaderDataType::Float3, "a_Pos" },
            { moon::ShaderDataType::Float4, "a_Color" }
        };
        vbuf->set_layout(layout);
        vertex_array_->add_vertex_buffer(vbuf);

        constexpr uint32_t indices[3] = { 0, 1, 2 };
        std::shared_ptr<moon::index_buffer> ibuf = std::shared_ptr<moon::index_buffer>(moon::index_buffer::create(indices, sizeof(indices) / sizeof(uint32_t)));
        vertex_array_->set_index_buffer(ibuf);

        // shader
        std::string vertex_shader_src = R"(
            #version 460 core
            layout (location = 0) in vec3 a_Pos;
            layout (location = 1) in vec4 a_Color;

            uniform mat4 u_VP;
            uniform mat4 u_Model;

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

        shader_ = std::make_shared<moon::shader>(vertex_shader_src, fragment_shader_src);

        square_va_ = std::shared_ptr<moon::vertex_array>(moon::vertex_array::create());
        constexpr float square_verts[3 * 4] = {
            -0.75f, -0.75f, 0.0f,
             0.75f, -0.75f, 0.0f,
             0.75f,  0.75f, 0.0f,
            -0.75f,  0.75f, 0.0f,
        };
        uint32_t square_indices[6] = {0, 1, 2, 2, 3, 0};
        std::shared_ptr<moon::vertex_buffer> square_vb = std::shared_ptr<moon::vertex_buffer>(moon::vertex_buffer::create(
            &square_verts[0], sizeof(square_verts)));
        square_vb->set_layout({
            { moon::ShaderDataType::Float3, "a_Pos" }
        });
        square_va_->add_vertex_buffer(square_vb);

        std::shared_ptr<moon::index_buffer> square_ib = std::shared_ptr<moon::index_buffer>(moon::index_buffer::create(
            &square_indices[0], sizeof(square_indices) / sizeof(uint32_t)));
        square_va_->set_index_buffer(square_ib);

        std::string blue_shader_vertex_src = R"(
            #version 460 core
            layout (location = 0) in vec3 a_Pos;

            uniform mat4 u_VP;
            uniform mat4 u_Model;

            void main()
            {
                gl_Position = u_VP * u_Model * vec4(a_Pos, 1.0);
            }
        )";
        std::string blue_shader_fragment_src = R"(
            #version 460 core
            out vec4 FragColor;

            void main()
            {
                FragColor = vec4(0.2, 0.3, 0.8, 1.0);
            }
        )";

        blue_shader_ = std::make_shared<moon::shader>(blue_shader_vertex_src, blue_shader_fragment_src);
    }

    void on_update() override
    {
        moon::render_command::set_clear_color( { 0.1f, 0.1f, 0.1f, 1.0f } );
        moon::render_command::clear();

        moon::renderer::begin_scene(camera_);

        blue_shader_->bind();
        moon::renderer::submit(square_va_);

        shader_->bind();
        moon::renderer::submit(vertex_array_);

        moon::renderer::end_scene();
    }

    void on_imgui_render() override
    {
        ImGui::Begin("test");
        ImGui::Text("Hello, world!");
        ImGui::End();
    }

    void on_event(moon::event& e) override
    {
        moon::event_dispatcher dispatcher(e);

        dispatcher.dispatch<moon::key_pressed_event>([&](moon::key_pressed_event& ke)
        {
            if (ke.get_keycode() == MOON_KEY_W)
            {
                camera_.translate({ 0.0f, 0.0f, -1.0f });
            }
            else if (ke.get_keycode() == MOON_KEY_S)
            {
                camera_.translate({ 0.0f, 0.0f, 1.0f });
            }
            else if (ke.get_keycode() == MOON_KEY_A)
            {
                camera_.translate({ -1.0f, 0.0f, 0.0f });
            }
            else if (ke.get_keycode() == MOON_KEY_D)
            {
                camera_.translate({ 1.0f, 0.0f, 0.0f });
            }
            return true;
        });
    }

private:
    std::shared_ptr<moon::vertex_array> vertex_array_;
    std::shared_ptr<moon::shader> shader_;

    std::shared_ptr<moon::vertex_array> square_va_;
    std::shared_ptr<moon::shader> blue_shader_;

    moon::camera camera_;
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
