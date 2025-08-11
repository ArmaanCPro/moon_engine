#pragma once

#include "moon/core/core.h"

#include "scene.h"
#include <entt/entt.hpp>

namespace moon
{
    class MOON_API entity
    {
    public:
        entity() = default;
        entity(entt::entity handle, scene* scene);
        entity(entity& other) noexcept = default;

        template <typename T>
        bool has_component() const
        {
            return m_scene_->m_registry_.any_of<T>(m_entity_handle_);
        }

        template <typename T>
        T& get_component()
        {
            MOON_CORE_ASSERT(has_component<T>(), "Entity does not have component!");

            return m_scene_->m_registry_.get<T>(m_entity_handle_);
        }

        template <typename T, typename... Args>
        T& add_component(Args&&... args)
        {
            MOON_CORE_ASSERT(!has_component<T>(), "Entity already has component!");

            // could also use emplace_or_replace and skip the assertion
            return m_scene_->m_registry_.emplace<T>(m_entity_handle_, std::forward<Args>(args)...);
        }

        template <typename T>
        void remove_component()
        {
            MOON_CORE_ASSERT(has_component<T>(), "Entity does not have component!");

            m_scene_->m_registry_.remove<T>(m_entity_handle_);
        }

        explicit operator bool() const { return m_entity_handle_ != entt::null; }

    private:
        entt::entity m_entity_handle_ {entt::null};
        // non-owning reference to a scene
        scene* m_scene_ = nullptr;
    };
}
