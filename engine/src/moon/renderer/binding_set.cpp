#include "moonpch.h"
#include "binding_set.h"

#include "renderer.h"
#include "platform/d3d12/d3d12_binding_set.h"

namespace moon
{
    void binding_layout::add_binding(const binding_element& element)
    {
        m_elements.push_back(element);
        validate_bindings();
    }

    void binding_layout::validate_bindings() const
    {
        // ensure no duplicate bindings within same set
        for (size_t i = 0; i < m_elements.size(); i++)
        {
            for (size_t j = i + 1; j < m_elements.size(); j++)
            {
                if (m_elements[i].binding == m_elements[j].binding
                    && m_elements[i].set == m_elements[j].set)
                {
                    MOON_CORE_ASSERT(false, "Duplicate binding in same set");
                }
            }
        }
    }

    ref<binding_set> binding_set::create(binding_layout layout, const ref<pipeline>& pipeline)
    {
        switch (renderer::get_api())
        {
        case renderer_api::API::None:
            MOON_CORE_ASSERT(false, "RendererAPI::None is not supported");
            return nullptr;
        case renderer_api::API::OpenGL:
            // TODO implement opengl binding_set
            return nullptr;
        case renderer_api::API::DirectX:
            return create_ref<d3d12_binding_set>(std::move(layout), pipeline);
        }

        MOON_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
}
