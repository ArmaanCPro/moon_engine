#include "core/events/key_event.h"
#include "core/events/mouse_event.h"

#include <moon.h>

#include <imgui.h>

class sandbox_layer : public moon::layer
{
public:
    sandbox_layer()
        :
        layer("sandbox"),
        camera_(-1.6f, 1.6f, -0.9f, 0.9f)
    {
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

        shader_ = std::make_shared<moon::shader>(vertex_shader_src, fragment_shader_src);

        square_va_ = std::shared_ptr<moon::vertex_array>(moon::vertex_array::create());
        constexpr float square_verts[3 * 4] = {
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
             0.5f,  0.5f, 0.0f,
            -0.5f,  0.5f, 0.0f,
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

            uniform vec4 u_Color;

            void main()
            {
                FragColor = u_Color;
            }
        )";

        flat_color_shader_ = std::make_shared<moon::shader>(flat_color_vertex_src, flat_color_fragment_src);
    }

    void on_update(moon::timestep ts) override
    {
        MOON_INFO("Delta time: {0} s ({1} ms)", ts.get_seconds(), ts.get_milliseconds());
        if (moon::input::is_key_pressed(MOON_KEY_W))
            cam_pos_.y += cam_move_speed_ * ts;
        if (moon::input::is_key_pressed(MOON_KEY_S))
            cam_pos_.y -= cam_move_speed_ * ts;
        if (moon::input::is_key_pressed(MOON_KEY_A))
            cam_pos_.x -= cam_move_speed_ * ts;
        if (moon::input::is_key_pressed(MOON_KEY_D))
            cam_pos_.x += cam_move_speed_ * ts;
        if (moon::input::is_key_pressed(MOON_KEY_Q))
            cam_rot_ += cam_rot_speed * ts;
        if (moon::input::is_key_pressed(MOON_KEY_E))
            cam_rot_ -= cam_rot_speed * ts;

        moon::render_command::set_clear_color({0.1f, 0.1f, 0.1f, 1.0f } );
        moon::render_command::clear();

        camera_.set_position(cam_pos_);
        camera_.set_rotation(cam_rot_);

        moon::renderer::begin_scene(camera_);

        static glm::vec4 red_c = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
        static glm::vec4 blue_c = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
        static glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
        for (int y = 0; y < 20; ++y)
        {
            for (int x = 0; x < 20; ++x)
            {
                glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
                glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
                if (x % 2 == 0)
                    flat_color_shader_->upload_uniform_float4("u_Color", red_c);
                else
                    flat_color_shader_->upload_uniform_float4("u_Color", blue_c);
                moon::renderer::submit(flat_color_shader_, square_va_, transform);
            }
        }
        //moon::renderer::submit(shader_, vertex_array_);

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
    }

private:
    std::shared_ptr<moon::vertex_array> vertex_array_;
    std::shared_ptr<moon::shader> shader_;

    std::shared_ptr<moon::vertex_array> square_va_;
    std::shared_ptr<moon::shader> flat_color_shader_;

    moon::ortho_camera camera_;
    glm::vec3 cam_pos_ {0.0f};
    float cam_move_speed_ = 2.5f;
    float cam_rot_ = 0.0f;
    float cam_rot_speed = 180.0f;
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
