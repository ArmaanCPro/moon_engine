#pragma once

#include "moon/core/core.h"

#include <string_view>
#include <unordered_map>

namespace moon
{
    class MOON_API shader
    {
    public:
        virtual ~shader() = default;

        virtual void bind() const = 0;
        virtual void unbind() const = 0;

        virtual std::string_view get_name() = 0;

        static ref<shader> create(std::string_view file_path);
        static ref<shader> create(std::string_view name, std::string_view vertex_src, std::string_view fragment_src);
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
