#pragma once

#include "moon/core/timestep.h"

#include "components.h"

#include <entt/entt.hpp>

namespace moon
{
    class entity;

    class MOON_API scene
    {
    public:
        scene();
        ~scene();

        entity create_entity(std::string_view name = "");

        void on_update(timestep ts);
        void on_viewport_resize(uint32_t width, uint32_t height);

    private:
        entt::registry m_registry_;
        uint32_t m_viewport_width_ = 0, m_viewport_height_ = 0;

        friend class entity;
    };
}
