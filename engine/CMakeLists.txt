cmake_minimum_required(VERSION 3.28)

project(moon_engine CXX)

if (MSVC)
    add_compile_options(/W4 /wd4201 /WX) #Warning level 4, all warnings are errors
else ()
    add_compile_options(-W -Wall -Werror) #All Warnings, all warnings are errors
endif ()

set(ENGINE_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/moonpch.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/application.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/log.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/layer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/layer_stack.cpp
        src/core/imgui/imgui_layer.cpp
)

set(ENGINE_HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/moonpch.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/moon.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/application.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/core.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/entry_point.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/events/application_event.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/events/event.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/events/key_event.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/events/mouse_event.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/log.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/window.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/layer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/layer_stack.h
        src/core/imgui/imgui_layer.h
        src/core/input.h
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

# Create a shared library instead of a static one
add_library(${PROJECT_NAME} SHARED ${ENGINE_SOURCES} ${ENGINE_HEADERS} )

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
find_package(spdlog CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC glfw glad::glad spdlog::spdlog_header_only imgui::imgui)
