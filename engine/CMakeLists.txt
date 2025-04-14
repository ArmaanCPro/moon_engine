cmake_minimum_required(VERSION 3.28)

project(moon_engine CXX)

if (MSVC)
    add_compile_options(/W4 /wd4201 /wd4100 /WX) #Warning level 4, all warnings are errors
else ()
    add_compile_options(-W -Wall -Werror -Wno-unused-parameter) #All Warnings, all warnings are errors
endif ()

set(ENGINE_SOURCES
        src/moonpch.cpp
        src/moon/application.cpp
        src/moon/log.cpp
        src/moon/layer.cpp
        src/moon/layer_stack.cpp
        src/moon/imgui/imgui_layer.cpp
        platform/opengl/opengl_context.cpp
        src/moon/renderer/shader.cpp
        src/moon/renderer/buffer.cpp
        platform/opengl/opengl_buffer.cpp
        src/moon/renderer/renderer.cpp
        src/moon/renderer/vertex_array.cpp
        platform/opengl/opengl_vertex_array.cpp
        src/moon/renderer/renderer_api.cpp
        src/moon/renderer/render_command.cpp
        platform/opengl/opengl_renderer_api.cpp
        src/moon/renderer/camera.cpp
        platform/opengl/opengl_shader.cpp
        src/moon/renderer/texture.cpp
        platform/opengl/opengl_texture.cpp
        src/moon/orthographic_camera_controller.cpp
)

set(ENGINE_HEADERS
        src/moonpch.h
        include/moon.h
        src/moon/application.h
        src/moon/core.h
        src/moon/entry_point.h
        src/moon/events/application_event.h
        src/moon/events/event.h
        src/moon/events/key_event.h
        src/moon/events/mouse_event.h
        src/moon/log.h
        src/moon/window.h
        src/moon/layer.h
        src/moon/layer_stack.h
        src/moon/imgui/imgui_layer.h
        src/moon/input.h
        src/moon/key_codes.h
        src/moon/mouse_codes.h
        src/moon/renderer/graphics_context.h
        platform/opengl/opengl_context.h
        src/moon/renderer/shader.h
        src/moon/renderer/buffer.h
        platform/opengl/opengl_buffer.h
        src/moon/renderer/renderer.h
        src/moon/renderer/vertex_array.h
        platform/opengl/opengl_vertex_array.h
        src/moon/renderer/renderer_api.h
        src/moon/renderer/render_command.h
        platform/opengl/opengl_renderer_api.h
        src/moon/renderer/camera.h
        src/moon/core/timestep.h
        platform/opengl/opengl_shader.h
        src/moon/renderer/texture.h
        platform/opengl/opengl_texture.h
        src/moon/orthographic_camera_controller.h
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
target_compile_definitions(${PROJECT_NAME} PRIVATE MOON_ENGINE_EXPORTS GLFW_INCLUDE_NONE)
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

# stb (image)
find_package(Stb REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${Stb_INCLUDE_DIR})

# cmrc
target_link_libraries(${PROJECT_NAME} PRIVATE textures::rc shaders::rc)
