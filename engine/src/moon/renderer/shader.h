#pragma once

#include "handle.h"
#include "moon/core/core.h"

#include <string_view>
#include <unordered_map>
#include <glm/glm.hpp>

#include "render_types.h"

namespace moon
{
    class MOON_API shader
    {
    public:
        virtual ~shader() = default;

        virtual void bind() const = 0;
        virtual void unbind() const = 0;

        virtual std::string_view get_name() = 0;
        virtual shader_module_handle get_handle() = 0;

        // TODO delete
        virtual void set_int(std::string_view name, int value) = 0;
        virtual void set_int_array(std::string_view name, int* values, uint32_t count) = 0;
        virtual void set_float(std::string_view name, float value) = 0;
        virtual void set_float2(std::string_view name, const glm::vec2& value) = 0;
        virtual void set_float3(std::string_view name, const glm::vec3& value) = 0;
        virtual void set_float4(std::string_view name, const glm::vec4& value) = 0;
        virtual void set_mat4(std::string_view name, const glm::mat4& value) = 0;

        static scope<shader> create(std::string_view file_path, ShaderStage stage);
        static scope<shader> create(std::string_view name, std::string_view src, ShaderStage shader_stage);
    };

    class MOON_API shader_library
    {
    public:
        shader_library() = default;
        ~shader_library() = default;

        void add(std::string_view name, const ref<shader>& shader);
        void add(const ref<shader>& shader);
        ref<shader> load(std::string_view file_path);
        ref<shader> load(std::string_view name, std::string_view filepath);

        ref<shader> get(std::string_view name);

        bool exists(std::string_view name);
    private:
        std::unordered_map<std::string, ref<shader>> shaders_;
    };
}
