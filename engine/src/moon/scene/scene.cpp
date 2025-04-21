#include "moonpch.h"
#include "scene.h"

#include "moon/renderer/renderer2d.h"

#include <glm/glm.hpp>

namespace moon
{
    scene::scene()
    {

    }

    scene::~scene()
    {

    }

    entt::entity scene::create_entity()
    {
        return m_registry_.create();
    }

    void scene::on_update(timestep ts)
    {
        auto group = m_registry_.group<transform_component>(entt::get<sprite_renderer_component>);
        for (auto entity : group)
        {
            auto [transform, sprite] = group.get<transform_component, sprite_renderer_component>(entity);
            //sprite.color = glm::sin(ts) * glm::vec4(1.0f);

            renderer2d::draw_quad(transform, sprite.color);
        }
    }
}
