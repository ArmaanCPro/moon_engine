#include "moonpch.h"
#include "scene.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace moon
{
    scene::scene()
    {
        struct transform_component
        {
            glm::mat4 transform = glm::mat4(1.0f);

            transform_component() = default;
            transform_component(const transform_component&) = default;
            explicit transform_component(const glm::mat4& transform)
            : transform(transform) {}

            operator glm::mat4&() { return transform; }
            operator const glm::mat4&() const { return transform; }
        };

        entt::entity entity = m_registry_.create();
        m_registry_.emplace<transform_component>(entity, glm::mat4(1.0f));

    }

    scene::~scene()
    {

    }
}
