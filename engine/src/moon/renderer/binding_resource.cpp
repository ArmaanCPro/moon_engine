#include "moonpch.h"
#include "binding_resource.h"

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
}
