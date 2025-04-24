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

    private:
        entt::registry m_registry_;

        friend class entity;
    };
}
