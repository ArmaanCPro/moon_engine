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

        glGenVertexArrays(1, &VAO_);
        glBindVertexArray(VAO_);

        constexpr float verts[3*3] = {
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
             0.0f,  0.5f, 0.0f
        };

        vertex_buffer_ = std::unique_ptr<vertex_buffer>(vertex_buffer::create(verts, sizeof(verts)));

        constexpr uint32_t indices[3] = { 0, 1, 2 };
        index_buffer_ = std::unique_ptr<index_buffer>(index_buffer::create(indices, sizeof(indices) / sizeof(uint32_t)));

        // shader
        std::string vertex_shader_src = R"(
            #version 460 core
            layout (location = 0) in vec3 a_Pos;

            out vec3 v_Pos;
            void main()
            {
                v_Pos = a_Pos + 0.5;
                gl_Position = vec4(a_Pos, 1.0);
            }
        )";
        std::string fragment_shader_src = R"(
            #version 460 core
            out vec4 FragColor;
            in vec3 v_Pos;
            void main()
            {
                FragColor = vec4(v_Pos * 0.5 + 0.5, 1.0f);
            }
        )";

        shader_ = std::make_unique<shader>(vertex_shader_src, fragment_shader_src);
    }

    application::~application()
    {
        glDeleteVertexArrays(1, &VAO_);
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

            shader_->bind();
            glBindVertexArray(VAO_);
            glDrawElements(GL_TRIANGLES, index_buffer_->get_count(), GL_UNSIGNED_INT, nullptr);

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
