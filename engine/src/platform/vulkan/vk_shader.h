#pragma once

#include "renderer/render_types.h"
#include "renderer/shader.h"
#include "vulkan/vk.h"

#include <filesystem>

namespace moon::vulkan
{
    class vk_context;

    class vk_shader final : public shader
    {
    public:
        explicit vk_shader(const std::filesystem::path& filepath, vk_context& context, ShaderStage stage);
        vk_shader(std::string_view source, vk_context& context, ShaderStage stage);
        ~vk_shader() override = default;

        // TODO names are outdated, use handle indexes instead
        std::string_view get_name() override { return {}; }

        shader_module_handle get_handle() override { return m_shader_module; }

        void bind() const override {}
        void unbind() const override {}

        void set_float(std::string_view name, float value) override {}
        void set_float2(std::string_view name, const glm::vec2& value) override {}
        void set_float3(std::string_view name, const glm::vec3& value) override {}
        void set_float4(std::string_view name, const glm::vec4& value) override {}
        void set_int(std::string_view name, int value) override {}
        void set_int_array(std::string_view name, int* values, uint32_t count) override {}
        void set_mat4(std::string_view name, const glm::mat4& value) override {}

    private:
        holder<shader_module_handle> m_shader_module;
        vk_context& m_context;
        ShaderStage m_stage;
    };
}
