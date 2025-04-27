#include "moonpch.h"
#include "scene.h"

#include "moon/renderer/renderer2d.h"
#include "entity.h"

#include <glm/glm.hpp>

namespace moon
{
    scene::scene()
    {

    }

    scene::~scene()
    {

    }

    entity scene::create_entity(std::string_view name)
    {
        entity e = { m_registry_.create(), this };
        e.add_component<transform_component>();
        e.add_component<tag_component>(name.empty() ? "Entity" : name);
        return e;
    }

    void scene::on_update(timestep ts)
    {
        // Render 2D
        const camera* main_camera = nullptr;
        const glm::mat4* camera_transform = nullptr;
        {
            auto group = m_registry_.group<camera_component>(entt::get<transform_component>);
            for (auto entity : group)
            {
                auto [camera, transform] = group.get<camera_component, transform_component>(entity);
                if (camera.primary)
                {
                    main_camera = &camera.camera;
                    camera_transform = &transform.transform;
                }
            }
        }

        if (main_camera && camera_transform)
        {
            renderer2d::begin_scene(*main_camera, *camera_transform);

            auto group = m_registry_.group<transform_component>(entt::get<sprite_renderer_component>);
            for (auto entity : group)
            {
                auto [transform, sprite] = group.get<transform_component, sprite_renderer_component>(entity);
                //sprite.color = glm::sin(ts) * glm::vec4(1.0f);

                renderer2d::draw_quad(transform, sprite.color);
            }

            renderer2d::end_scene();
        }
    }

    void scene::on_viewport_resize(uint32_t width, uint32_t height)
    {
        m_viewport_width_ = width;
        m_viewport_height_ = height;

        // resize non-fixed aspect ratio cameras
        auto view = m_registry_.view<camera_component>();
        for (auto entity : view)
        {
            auto& camera_comp = view.get<camera_component>(entity);
            if (!camera_comp.fixed_aspect_ratio)
            {
                //camera_comp.camera.set_viewport(0, 0, width, height);
            }
        }
    }
}
