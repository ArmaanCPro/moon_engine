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

    static GLenum shader_data_type_to_gl_type(ShaderDataType type)
    {
        switch (type)
        {
            case ShaderDataType::Float:     return GL_FLOAT;
            case ShaderDataType::Float2:    return GL_FLOAT;
            case ShaderDataType::Float3:    return GL_FLOAT;
            case ShaderDataType::Float4:    return GL_FLOAT;
            case ShaderDataType::Mat3:      return GL_FLOAT;
            case ShaderDataType::Mat4:      return GL_FLOAT;
            case ShaderDataType::Int:       return GL_INT;
            case ShaderDataType::Int2:      return GL_INT;
            case ShaderDataType::Int3:      return GL_INT;
            case ShaderDataType::Int4:      return GL_INT;
            case ShaderDataType::Bool:      return GL_BOOL;
            default: MOON_CORE_ASSERT(false, "Unknown ShaderDataType!"); return 0;
        }
    }

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

        constexpr float verts[3 * 7] = {
            -0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.7f, 1.0f,
             0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.85f, 1.0f,
             0.0f,  0.5f, 0.0f, 0.7f, 0.8f, 0.2f, 1.0f,
        };

        vertex_buffer_ = std::unique_ptr<vertex_buffer>(vertex_buffer::create(verts, sizeof(verts)));

        {
            buffer_layout layout = {
                { ShaderDataType::Float3, "a_Pos" },
                { ShaderDataType::Float4, "a_Color" }
            };
            vertex_buffer_->set_layout(layout);
        }

        uint32_t buffer_index = 0;
        const auto& layout = vertex_buffer_->get_layout();
        for (const auto& element : layout)
        {
            glVertexAttribPointer(buffer_index,
                element.get_component_count(),
                shader_data_type_to_gl_type(element.type),
                element.normalized ? GL_TRUE : GL_FALSE,
                layout.get_stride(),
                (const void*)element.offset
            );

            glEnableVertexAttribArray(buffer_index);
            buffer_index++;
        }

        constexpr uint32_t indices[3] = { 0, 1, 2 };
        index_buffer_ = std::unique_ptr<index_buffer>(index_buffer::create(indices, sizeof(indices) / sizeof(uint32_t)));

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
