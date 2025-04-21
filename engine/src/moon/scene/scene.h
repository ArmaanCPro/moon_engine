#pragma once

#include "moon/core/timestep.h"

#include "components.h"

#include <entt/entt.hpp>

namespace moon
{
    class MOON_API scene
    {
    public:
        scene();
        ~scene();

        entt::entity create_entity();

        // temp
        entt::registry& reg() { return m_registry_; }

        void on_update(timestep ts);

    private:
        entt::registry m_registry_;

    };
}
