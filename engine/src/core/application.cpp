#include "moonpch.h"

#include "application.h"

#include "log.h"
#include "events/application_event.h"
#include "events/event.h"

#include "imgui/imgui_layer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <memory>

namespace moon
{
    application* application::s_instance = nullptr;

    application::application()
    {
        MOON_CORE_ASSERT(!s_instance, "Application already exists!");
        s_instance = this;

        window_ = std::unique_ptr<window>(window::create());
        window_->set_event_callback([&](event& e) { on_event(e); });

        imgui_layer_ = new imgui_layer();
        push_layer(imgui_layer_);

        vertex_array_ = std::shared_ptr<vertex_array>(vertex_array::create());

        constexpr float verts[3 * 7] = {
            -0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.7f, 1.0f,
             0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.85f, 1.0f,
             0.0f,  0.5f, 0.0f, 0.7f, 0.8f, 0.2f, 1.0f,
        };
        std::shared_ptr<vertex_buffer> vbuf = std::shared_ptr<vertex_buffer>(vertex_buffer::create(verts, sizeof(verts)));
        buffer_layout layout = {
            { ShaderDataType::Float3, "a_Pos" },
            { ShaderDataType::Float4, "a_Color" }
        };
        vbuf->set_layout(layout);
        vertex_array_->add_vertex_buffer(vbuf);

        constexpr uint32_t indices[3] = { 0, 1, 2 };
        std::shared_ptr<index_buffer> ibuf = std::shared_ptr<index_buffer>(index_buffer::create(indices, sizeof(indices) / sizeof(uint32_t)));
        vertex_array_->set_index_buffer(ibuf);

        // shader
        std::string vertex_shader_src = R"(
            #version 460 core
            layout (location = 0) in vec3 a_Pos;
            layout (location = 1) in vec4 a_Color;

            out vec4 v_Color;
            void main()
            {
                v_Color = a_Color;
                gl_Position = vec4(a_Pos, 1.0);
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

        shader_ = std::make_shared<shader>(vertex_shader_src, fragment_shader_src);

        square_va_ = std::shared_ptr<vertex_array>(vertex_array::create());
        constexpr float square_verts[3 * 4] = {
            -0.75f, -0.75f, 0.0f,
             0.75f, -0.75f, 0.0f,
             0.75f,  0.75f, 0.0f,
            -0.75f,  0.75f, 0.0f,
        };
        uint32_t square_indices[6] = {0, 1, 2, 2, 3, 0};
        std::shared_ptr<vertex_buffer> square_vb = std::shared_ptr<vertex_buffer>(vertex_buffer::create(
            &square_verts[0], sizeof(square_verts)));
        square_vb->set_layout({
            { ShaderDataType::Float3, "a_Pos" }
        });
        square_va_->add_vertex_buffer(square_vb);

        std::shared_ptr<index_buffer> square_ib = std::shared_ptr<index_buffer>(index_buffer::create(
            &square_indices[0], sizeof(square_indices) / sizeof(uint32_t)));
        square_va_->set_index_buffer(square_ib);


        std::string blue_shader_vertex_src = R"(
            #version 460 core
            layout (location = 0) in vec3 a_Pos;

            void main()
            {
                gl_Position = vec4(a_Pos, 1.0);
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

        blue_shader_ = std::make_shared<shader>(blue_shader_vertex_src, blue_shader_fragment_src);
    }

    application::~application()
    {

    }

    void application::push_layer(layer* layer)
    {
        layer_stack_.push_layer(layer);
    }

    void application::push_overlay(layer* layer)
    {
        layer_stack_.push_overlay(layer);
    }

    void application::run()
    {
        while (running_)
        {
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            blue_shader_->bind();
            square_va_->bind();
            glDrawElements(GL_TRIANGLES, (GLsizei)square_va_->get_index_buffer()->get_count(), GL_UNSIGNED_INT, nullptr);

            shader_->bind();
            vertex_array_->bind();
            glDrawElements(GL_TRIANGLES, (GLsizei)vertex_array_->get_index_buffer()->get_count(), GL_UNSIGNED_INT, nullptr);

            for (layer* l : layer_stack_)
                l->on_update();

            imgui_layer_->begin();
            for (layer* l : layer_stack_)
            {
                l->on_imgui_render();
            }
            imgui_layer_->end();

            window_->on_update();
        }
    }

    void application::on_event(event& e)
    {
        event_dispatcher dispatcher(e);
        dispatcher.dispatch<window_close_event>([&](window_close_event& wce) { return on_window_close(wce); });

        for (auto it = layer_stack_.end(); it != layer_stack_.begin(); )
        {
            (*--it)->on_event(e);
            if (e.handled)
                break;
        }
    }

    bool application::on_window_close(window_close_event&)
    {
        running_ = false;
        return true;
    }
}
