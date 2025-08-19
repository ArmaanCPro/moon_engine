#pragma once

#include <vector>

namespace moon
{
    enum class ShaderDataType : uint8_t
    {
        None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
    };

    static uint32_t shader_data_type_size(ShaderDataType type)
    {
        switch (type)
        {
        case ShaderDataType::Float:    return 4;
        case ShaderDataType::Float2:   return 4 * 2;
        case ShaderDataType::Float3:   return 4 * 3;
        case ShaderDataType::Float4:   return 4 * 4;
        case ShaderDataType::Mat3:     return 4 * 3 * 3;
        case ShaderDataType::Mat4:     return 4 * 4 * 4;
        case ShaderDataType::Int:      return 4;
        case ShaderDataType::Int2:     return 4 * 2;
        case ShaderDataType::Int3:     return 4 * 3;
        case ShaderDataType::Int4:     return 4 * 4;
        case ShaderDataType::Bool:     return 1;
        default:
            MOON_CORE_ASSERT_MSG(false, "Unknown ShaderDataType!");
            return 0;
        }
    }

    struct MOON_API buffer_element
    {
        ShaderDataType type;
        std::string name;
        uint32_t size;
        uint64_t offset {0};
        bool normalized;

        buffer_element() = default;
        buffer_element(ShaderDataType type, const std::string& name, bool normalized = false)
            :
            type(type), name(name), size(shader_data_type_size(type)), normalized(normalized)
        {}

        uint32_t get_component_count() const
        {
            switch (type)
            {
                case ShaderDataType::Float:    return 1;
                case ShaderDataType::Float2:   return 2;
                case ShaderDataType::Float3:   return 3;
                case ShaderDataType::Float4:   return 4;
                case ShaderDataType::Mat3:     return 3 * 3;
                case ShaderDataType::Mat4:     return 4 * 4;
                case ShaderDataType::Int:      return 1;
                case ShaderDataType::Int2:     return 2;
                case ShaderDataType::Int3:     return 3;
                case ShaderDataType::Int4:     return 4;
                case ShaderDataType::Bool:     return 1;
                default:
                    MOON_CORE_ASSERT_MSG(false, "Unknown ShaderDataType!"); return 0;
            }
        }
    };

    class MOON_API buffer_layout
    {
    public:
        buffer_layout() = default;
        buffer_layout(const std::initializer_list<buffer_element>& elements)
            :
            elements_(elements)
        {
            calc_offsets_and_stride();
        }
        inline const std::vector<buffer_element>& get_elements() const { return elements_; }
        inline uint32_t get_stride() const { return stride_; }

        std::vector<buffer_element>::iterator begin() { return elements_.begin(); }
        std::vector<buffer_element>::iterator end() { return elements_.end(); }
        std::vector<buffer_element>::const_iterator begin() const { return elements_.begin(); }
        std::vector<buffer_element>::const_iterator end() const { return elements_.end(); }

    private:
        void calc_offsets_and_stride()
        {
            uint32_t offset = 0;
            stride_ = 0;
            for (auto& element : elements_)
            {
                element.offset = offset;
                offset += element.size;
            }
            stride_ = offset;
        }
        std::vector<buffer_element> elements_;
        uint32_t stride_{0};
    };

    class MOON_API vertex_buffer
    {
    public:
        virtual ~vertex_buffer() = default;

        virtual void bind() const = 0;
        virtual void unbind() const = 0;

        virtual void set_data(const void* data, uint32_t size) = 0;

        virtual const buffer_layout& get_layout() const = 0;
        virtual void set_layout(const buffer_layout& layout) = 0;

        static ref<vertex_buffer> create(uint32_t size);
        static ref<vertex_buffer> create(const float* vertices, uint32_t size);
    };

    // Currently, only 32-bit index buffers are supported
    class MOON_API index_buffer
    {
    public:
        virtual ~index_buffer() = default;

        virtual uint32_t get_count() const = 0;

        virtual void bind() const = 0;
        virtual void unbind() const = 0;

        static ref<index_buffer> create(const uint32_t* indices, uint32_t count);
    };
}
