#include "moonpch.h"
#include "entity.h"

namespace moon
{
    entity::entity(entt::entity handle, scene* scene)
        :
        m_entity_handle_(handle),
        m_scene_(scene)
    {}
}
