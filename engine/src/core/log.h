#pragma once

#include "core.h"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace moon
{
    class log
    {
    public:
        static MOON_API void init();

        inline static std::shared_ptr<spdlog::logger>& get_core_logger()
        {
            return s_core_logger;
        }

        inline static std::shared_ptr<spdlog::logger>& get_client_logger()
        {
            return s_client_logger;
        }

    private:
        static MOON_API std::shared_ptr<spdlog::logger> s_core_logger;
        static MOON_API std::shared_ptr<spdlog::logger> s_client_logger;
    };
}

// Core log macros
#define MOON_CORE_TRACE(...)    ::moon::log::get_core_logger()->trace(__VA_ARGS__)
#define MOON_CORE_INFO(...)     ::moon::log::get_core_logger()->info(__VA_ARGS__)
#define MOON_CORE_WARN(...)     ::moon::log::get_core_logger()->warn(__VA_ARGS__)
#define MOON_CORE_ERROR(...)    ::moon::log::get_core_logger()->error(__VA_ARGS__)
#define MOON_CORE_FATAL(...)    ::moon::log::get_core_logger()->critical(__VA_ARGS__)

// Client log macros
#define MOON_TRACE(...)         ::moon::log::get_client_logger()->trace(__VA_ARGS__)
#define MOON_INFO(...)          ::moon::log::get_client_logger()->info(__VA_ARGS__)
#define MOON_WARN(...)          ::moon::log::get_client_logger()->warn(__VA_ARGS__)
#define MOON_ERROR(...)         ::moon::log::get_client_logger()->error(__VA_ARGS__)
#define MOON_FATAL(...)         ::moon::log::get_client_logger()->critical(__VA_ARGS__)
