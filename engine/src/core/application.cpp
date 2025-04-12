#include "moonpch.h"

#include "application.h"

#include "log.h"
#include "events/application_event.h"
#include "events/event.h"

#include "imgui/imgui_layer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

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

        glGenBuffers(1, &VBO_);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_);

        float verts[3*3] = {
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
             0.0f,  0.5f, 0.0f
        };

        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);

        glGenBuffers(1, &IBO_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO_);

        unsigned int indices[3] = { 0, 1, 2 };
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // shader
        const char* vertexShaderSource = "#version 460 core\n"
            "layout (location = 0) in vec3 aPos;\n"
            "void main()\n"
            "{\n"
            "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
            "}\0";
        const char* fragmentShaderSource = "#version 460 core\n"
            "out vec4 FragColor;\n"
            "void main()\n"
            "{\n"
            "   FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
            "}\0";

        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
        glCompileShader(vertexShader);

        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
        glCompileShader(fragmentShader);

        shader_program_ = glCreateProgram();
        glAttachShader(shader_program_, vertexShader);
        glAttachShader(shader_program_, fragmentShader);
        glLinkProgram(shader_program_);

        // Clean up
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

    }

    application::~application()
    {
        glDeleteBuffers(1, &VBO_);
        glDeleteBuffers(1, &IBO_);
        glDeleteVertexArrays(1, &VAO_);
        glDeleteProgram(shader_program_);
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
            glUseProgram(shader_program_);

            glBindVertexArray(VAO_);
            glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);

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
