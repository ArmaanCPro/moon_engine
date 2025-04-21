#include "editor_layer.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace moon
{
    editor_layer::editor_layer()
        :
        layer("Moon Engine"),
        m_camera_controller_(16.0f / 9.0f, true)
    {}

    void editor_layer::on_attach()
    {
        MOON_PROFILE_FUNCTION();

        m_checkerboard_texture_ = texture2d::create("assets/textures/Checkerboard.png");

        framebuffer_spec fb_spec;
        fb_spec.width = 1280;
        fb_spec.height = 720;
        m_framebuffer_ = framebuffer::create(fb_spec);
    }

    void editor_layer::on_detach()
    {
        MOON_PROFILE_FUNCTION();

    }

    void editor_layer::on_update(timestep ts)
    {
        MOON_PROFILE_FUNCTION();

        m_camera_controller_.on_update(ts);

        renderer2d::reset_stats();
        {
            MOON_PROFILE_SCOPE("Renderer Prep");

            m_framebuffer_->bind();
            render_command::set_clear_color({0.1f, 0.1f, 0.1f, 1.0f } );
            render_command::clear();
        }

        {
            static float rotation = 0.0f;
            rotation += ts * 50.0f;

            MOON_PROFILE_SCOPE("Renderer Draw");
            renderer2d::begin_scene(m_camera_controller_.get_camera());

            renderer2d::draw_rotated_quad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, glm::radians(45.0f), { 0.2f, 0.1f, 0.8f, 1.0f });
            renderer2d::draw_quad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, { 0.2f, 0.1f, 0.8f, 1.0f });
            renderer2d::draw_quad({ 0.5f, -0.5f }, { 0.5f, 0.75f }, m_square_color_);
            renderer2d::draw_quad({ 0.0f, 0.0f, -0.1f }, glm::vec2(20.0f), m_checkerboard_texture_, 10.0f);
            renderer2d::draw_rotated_quad({ -2.0f, 0.0 }, glm::vec2(1.0f), glm::radians(rotation), m_checkerboard_texture_, 10.0f);

            for (float y = -5.0f; y < 5.0f; y += 0.5f)
            {
                for (float x = -5.0f; x < 5.0f; x += 0.5f)
                {
                    glm::vec4 color = { (x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.7f };
                    renderer2d::draw_quad({ x, y }, { 0.45f, 0.45f }, color);
                }
            }

            renderer2d::end_scene();
            m_framebuffer_->unbind();
        }
    }

    void editor_layer::on_imgui_render()
    {
        MOON_PROFILE_FUNCTION();

        if (!ImGui::GetCurrentContext())
            ImGui::SetCurrentContext(moon_get_imgui_context());

        static bool dockspace_open = true;
        static bool opt_fullscreen_persistant = true;
        bool opt_fullscreen = opt_fullscreen_persistant;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen)
        {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        }

        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Demo", &dockspace_open, window_flags);
        ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        // DockSpace
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Exit", "", (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0))
                    application::get().close();

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        ImGui::Begin("Settings");

        auto stats = renderer2d::get_stats();
        ImGui::Text("Renderer2D Stats:");
        ImGui::Text("Draw Calls: %d", stats.draw_calls);
        ImGui::Text("Quads: %d", stats.quad_count);
        ImGui::Text("Vertices: %d", stats.get_total_vertex_count());
        ImGui::Text("Indices: %d", stats.get_total_index_count());
        ImGui::ColorEdit4("Square Color", glm::value_ptr(m_square_color_));
        ImGui::End();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport");
        ImVec2 viewport_panel_size = ImGui::GetContentRegionAvail();
        if (m_viewport_size_ != *((glm::vec2*)&viewport_panel_size))
        {
            m_framebuffer_->resize((uint32_t)viewport_panel_size.x, (uint32_t)viewport_panel_size.y);
            m_viewport_size_ = { viewport_panel_size.x, viewport_panel_size.y };

            m_camera_controller_.on_resize(viewport_panel_size.x, viewport_panel_size.y);
        }

        const uint32_t texture_id = m_framebuffer_->get_color_attachment_renderer_id();
        ImGui::Image(texture_id, { m_viewport_size_.x, m_viewport_size_.y }, { 0, 1 }, { 1, 0 });
        ImGui::End();
        ImGui::PopStyleVar();

        ImGui::End();
    }

    void editor_layer::on_event(event& e)
    {
        m_camera_controller_.on_event(e);
    }
}
