cmake_minimum_required(VERSION 3.28)

project(moon_engine CXX)

if (MSVC)
    add_compile_options(/W4 /wd4201 /WX) #Warning level 4, all warnings are errors
else ()
    add_compile_options(-W -Wall -Werror) #All Warnings, all warnings are errors
endif ()

set(ENGINE_SOURCES
        src/moonpch.cpp
        src/core/application.cpp
        src/core/log.cpp
        src/core/layer.cpp
        src/core/layer_stack.cpp
        src/core/imgui/imgui_layer.cpp
        platform/opengl/opengl_context.cpp
        src/core/renderer/shader.cpp
        src/core/renderer/buffer.cpp
        platform/opengl/opengl_buffer.cpp
        src/core/renderer/renderer.cpp
        src/core/renderer/vertex_array.cpp
        platform/opengl/opengl_vertex_array.cpp
        src/core/renderer/renderer_api.cpp
        src/core/renderer/render_command.cpp
        platform/opengl/opengl_renderer_api.cpp
)

set(ENGINE_HEADERS
        src/moonpch.h
        include/moon.h
        src/core/application.h
        src/core/core.h
        src/core/entry_point.h
        src/core/events/application_event.h
        src/core/events/event.h
        src/core/events/key_event.h
        src/core/events/mouse_event.h
        src/core/log.h
        src/core/window.h
        src/core/layer.h
        src/core/layer_stack.h
        src/core/imgui/imgui_layer.h
        src/core/input.h
        src/core/key_codes.h
        src/core/mouse_codes.h
        src/core/renderer/graphics_context.h
        platform/opengl/opengl_context.h
        src/core/renderer/shader.h
        src/core/renderer/buffer.h
        platform/opengl/opengl_buffer.h
        src/core/renderer/renderer.h
        src/core/renderer/vertex_array.h
        platform/opengl/opengl_vertex_array.h
        src/core/renderer/renderer_api.h
        src/core/renderer/render_command.h
        platform/opengl/opengl_renderer_api.h
)

# platform
if (WIN32 OR MINGW)
    SET(PLATFORM_PREFIX windows)
elseif (APPLE)
    SET(PLATFORM_PREFIX apple)
elseif (UNIX)
    SET(PLATFORM_PREFIX linux)
endif ()
set(PLATFORM_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/platform/${PLATFORM_PREFIX}/${PLATFORM_PREFIX}_window.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/platform/${PLATFORM_PREFIX}/${PLATFORM_PREFIX}_input.cpp"
)
set(PLATFORM_HEADERS
        "${CMAKE_CURRENT_SOURCE_DIR}/platform/${PLATFORM_PREFIX}/${PLATFORM_PREFIX}_window.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/platform/${PLATFORM_PREFIX}/${PLATFORM_PREFIX}_input.h"
)
list(APPEND ENGINE_SOURCES ${PLATFORM_SOURCES})
list(APPEND ENGINE_HEADERS ${PLATFORM_HEADERS})

# Create a static lib
add_library(${PROJECT_NAME} STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS} )

# compile definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE MOON_IS_MONOLITHIC MOON_ENGINE_EXPORTS GLFW_INCLUDE_NONE)
if (WIN32 OR MINGW)
    target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX _CRT_SECURE_NO_WARNINGS WIN32_LEAN_AND_MEAN)
endif ()

# Set include directories
target_include_directories(${PROJECT_NAME}
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/platform
)

# pch
target_precompile_headers(${PROJECT_NAME} PRIVATE src/moonpch.h)

# libraries
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC spdlog::spdlog_header_only fmt::fmt-header-only imgui::imgui)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw glad::glad glm::glm-header-only)
