cmake_minimum_required(VERSION 3.28)

project(moon_engine CXX)

if (MSVC)
    add_compile_options(/W4 /wd4201 /wd4100 /WX) #Warning level 4, all warnings are errors
else ()
    add_compile_options(-W -Wall -Werror -Wno-unused-parameter) #All Warnings, all warnings are errors
endif ()

set(ENGINE_SOURCES
        src/moonpch.cpp
        src/moon/core/application.cpp
        src/moon/core/log.cpp
        src/moon/core/layer.cpp
        src/moon/core/layer_stack.cpp
        src/moon/imgui/imgui_layer.cpp
        src/platform/opengl/opengl_context.cpp
        src/moon/renderer/shader.cpp
        src/moon/renderer/buffer.cpp
        src/platform/opengl/opengl_buffer.cpp
        src/moon/renderer/renderer.cpp
        src/moon/renderer/vertex_array.cpp
        src/platform/opengl/opengl_vertex_array.cpp
        src/moon/renderer/renderer_api.cpp
        src/moon/renderer/render_command.cpp
        src/platform/opengl/opengl_renderer_api.cpp
        src/moon/renderer/camera.cpp
        src/platform/opengl/opengl_shader.cpp
        src/moon/renderer/texture.cpp
        src/platform/opengl/opengl_texture.cpp
        src/moon/renderer/orthographic_camera_controller.cpp
        src/moon/renderer/renderer2d.cpp
        src/moon/renderer/subtexture2d.cpp
        src/moon/renderer/framebuffer.cpp
        src/platform/opengl/opengl_framebuffer.cpp
        src/moon/scene/scene.cpp
        src/moon/scene/entity.cpp
        src/platform/directx12/directx_context.cpp
        src/platform/directx12/directx_renderer_api.cpp
        src/platform/directx12/directx_vertex_array.cpp
        src/moon/renderer/pipeline.cpp
        src/platform/directx12/directx_pipeline.cpp
        src/platform/directx12/directx_shader.cpp
        src/platform/directx12/directx_texture.cpp
        src/platform/directx12/directx_imgui_layer.cpp
)

set(ENGINE_HEADERS
        src/moonpch.h
        include/moon.h
        src/moon/core/application.h
        src/moon/core/core.h
        src/moon/core/entry_point.h
        src/moon/events/application_event.h
        src/moon/events/event.h
        src/moon/events/key_event.h
        src/moon/events/mouse_event.h
        src/moon/core/log.h
        src/moon/core/window.h
        src/moon/core/layer.h
        src/moon/core/layer_stack.h
        src/moon/imgui/imgui_layer.h
        src/moon/core/input.h
        src/moon/core/key_codes.h
        src/moon/core/mouse_codes.h
        src/moon/renderer/graphics_context.h
        src/platform/opengl/opengl_context.h
        src/moon/renderer/shader.h
        src/moon/renderer/buffer.h
        src/platform/opengl/opengl_buffer.h
        src/moon/renderer/renderer.h
        src/moon/renderer/vertex_array.h
        src/platform/opengl/opengl_vertex_array.h
        src/moon/renderer/renderer_api.h
        src/moon/renderer/render_command.h
        src/platform/opengl/opengl_renderer_api.h
        src/moon/renderer/camera.h
        src/moon/core/timestep.h
        src/platform/opengl/opengl_shader.h
        src/moon/renderer/texture.h
        src/platform/opengl/opengl_texture.h
        src/moon/renderer/orthographic_camera_controller.h
        src/moon/renderer/renderer2d.h
        src/moon/debug/instrumentor.h
        src/moon/renderer/SubTexture2D.h
        src/moon/renderer/framebuffer.h
        src/platform/opengl/opengl_framebuffer.h
        src/moon/scene/scene.h
        src/moon/scene/components.h
        src/moon/scene/entity.h
        src/platform/directx12/directx_context.h
        src/platform/directx12/directx_renderer_api.h
        src/platform/directx12/directx.h
        src/platform/directx12/directx_buffer.h
        src/platform/directx12/directx_buffer.cpp
        src/platform/directx12/directx_vertex_array.h
        src/moon/renderer/pipeline.h
        src/platform/directx12/directx_pipeline.h
        src/platform/directx12/directx_shader.h
        src/platform/directx12/directx_texture.h
        src/platform/directx12/directx_imgui_layer.h
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
        "src/platform/${PLATFORM_PREFIX}/${PLATFORM_PREFIX}_window.cpp"
        "src/platform/${PLATFORM_PREFIX}/${PLATFORM_PREFIX}_input.cpp"
)
set(PLATFORM_HEADERS
        "src/platform/${PLATFORM_PREFIX}/${PLATFORM_PREFIX}_window.h"
)
list(APPEND ENGINE_SOURCES ${PLATFORM_SOURCES})
list(APPEND ENGINE_HEADERS ${PLATFORM_HEADERS})

# generate static/shared lib based on IS_MONOLITHIC option
if (IS_MONOLITHIC)
    add_library(${PROJECT_NAME} STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})
else ()
    add_library(${PROJECT_NAME} SHARED ${ENGINE_SOURCES} ${ENGINE_HEADERS})
endif ()

# compile definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE MOON_ENGINE_EXPORTS GLFW_INCLUDE_NONE)

# Set include directories
target_include_directories(${PROJECT_NAME}
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/moon
        ${CMAKE_CURRENT_SOURCE_DIR}src/platform
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
find_package(EnTT CONFIG REQUIRED)
find_package(directx-headers CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC spdlog::spdlog_header_only fmt::fmt-header-only imgui::imgui glm::glm-header-only EnTT::EnTT)

target_link_libraries(${PROJECT_NAME} PUBLIC glfw glad::glad Microsoft::DirectX-Headers)

# stb (image)
find_package(Stb REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${Stb_INCLUDE_DIR})

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE d3d12 dxgi d3dcompiler dxguid)
endif ()
