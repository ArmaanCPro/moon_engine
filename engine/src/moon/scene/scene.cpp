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
        auto group = m_registry_.group<transform_component>(entt::get<sprite_renderer_component>);
        for (auto entity : group)
        {
            auto [transform, sprite] = group.get<transform_component, sprite_renderer_component>(entity);
            //sprite.color = glm::sin(ts) * glm::vec4(1.0f);

            renderer2d::draw_quad(transform, sprite.color);
        }
    }
}
