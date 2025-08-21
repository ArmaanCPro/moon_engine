#include "moonpch.h"
#include "vk_shader.h"

#include "vk_context.h"

#include <fstream>

namespace moon::vulkan
{
    vk_shader::vk_shader(const std::filesystem::path& filepath, vk_context& context, ShaderStage stage)
        :
        m_context(context)
        , m_stage(stage)
    {
        std::string source = "";

        std::ifstream file(absolute(filepath), std::ios::ate | std::ios::binary);
        MOON_CORE_ASSERT_MSG(file, "Failed to open file!");

        auto size = file.tellg();
        file.seekg(0, std::ios::beg);

        source.resize(size);
        file.read(&source[0], size);
        file.close();

        m_shader_module = m_context.create_shader_module({
            source.c_str(),
            stage,
            filepath.string().c_str()
        }).value();
    }

    // TODO consider making this const std::string& because create_shader_module expects c string
    vk_shader::vk_shader(std::string_view source, vk_context& context, ShaderStage stage)
        :
        m_shader_module(context.create_shader_module({
            source.data(), stage, "vk_shader"
        }).value())
        , m_context(context)
        , m_stage(stage)
    {}
}
