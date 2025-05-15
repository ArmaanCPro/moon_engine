#pragma once

#include <utility>

#include "buffer.h"
#include "shader.h"
#include "texture.h"
#include "moon/core/core.h"

namespace moon
{
    enum class BindingResourceType : uint8_t
    {
        UniformBuffer,
        StorageBuffer,
        Texture,
        Sampler,
        CombinedTextureSampler
    };

    struct MOON_API binding_element
    {
        BindingResourceType type;
        uint32_t binding;
        uint32_t set;
        std::string name;
        uint32_t array_size = 1;

        binding_element() = default;
        binding_element(BindingResourceType type, uint32_t binding, uint32_t set, std::string_view name, uint32_t array_size = 1)
            :
            type(type), binding(binding), set(set), name(name), array_size(array_size)
        {}
    };

    class MOON_API binding_layout
    {
    public:
        binding_layout() = default;
        binding_layout(const std::initializer_list<binding_element>& elements)
            :
            m_elements(elements)
        {
            validate_bindings();
        }

        void add_binding(const binding_element& element);

        inline const std::vector<binding_element>& get_elements() const { return m_elements; }

        // iterators
        std::vector<binding_element>::iterator begin() { return m_elements.begin(); }
        std::vector<binding_element>::iterator end() { return m_elements.end(); }
        std::vector<binding_element>::const_iterator begin() const { return m_elements.begin(); }
        std::vector<binding_element>::const_iterator end() const { return m_elements.end(); }

    private:
        void validate_bindings() const;

        std::vector<binding_element> m_elements;
    };


    class pipeline;

    class MOON_API binding_set
    {
    public:
        virtual ~binding_set() = default;

        virtual void bind() const = 0;
        virtual void unbind() const = 0;

        const binding_layout& get_layout() const { return m_layout; }

        // resource setting TODO: remove these from shader class
        virtual void set_uniform_buffer(uint32_t binding, const ref<vertex_buffer>& buffer) = 0;
        virtual void set_constant(uint32_t binding, const void* data, size_t size) = 0;
        virtual void set_texture(uint32_t binding, const ref<texture2d>& texture) = 0;

        static ref<binding_set> create(binding_layout layout, const ref<pipeline>& pipeline);

    protected:
        explicit binding_set(binding_layout layout)
            :
            m_layout(std::move(layout))
        {}

    protected:
        binding_layout m_layout;
    };
}
