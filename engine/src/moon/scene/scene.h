#pragma once

#include <entt/entt.hpp>

namespace moon
{
    class MOON_API scene
    {
    public:
        scene();
        ~scene();

    private:
        entt::registry m_registry_;

    };
}
