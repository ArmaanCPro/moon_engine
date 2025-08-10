cmake_minimum_required(VERSION 3.28)

project(moon_engine CXX)

if (MSVC)
    add_compile_options(/W4 /wd4201 /wd4100 /WX) #Warning level 4, all warnings are errors
else ()
    add_compile_options(-W -Wall -Werror -Wno-unused-parameter) #All Warnings, all warnings are errors
endif ()



# platform
if (WIN32 OR MINGW)
    SET(PLATFORM_PREFIX windows)
elseif (APPLE)
    SET(PLATFORM_PREFIX apple)
elseif (UNIX)
    SET(PLATFORM_PREFIX linux)
endif ()

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
        "${PROJECT_SOURCE_DIR}/src/moon/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/platform/opengl/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/platform/vulkan/*.cpp"
        "src/platform/${PLATFORM_PREFIX}/*.cpp"
)
file(GLOB_RECURSE ENGINE_HEADERS CONFIGURE_DEPENDS
        "${PROJECT_SOURCE_DIR}/src/moon/*.h"
        "${PROJECT_SOURCE_DIR}/include/*.h"
        "${PROJECT_SOURCE_DIR}/src/platform/opengl/*.h"
        "${PROJECT_SOURCE_DIR}/src/platform/vulkan/*.h"
        "src/platform/${PLATFORM_PREFIX}/*.h"
)

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
        ${CMAKE_CURRENT_SOURCE_DIR}/src/platform
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
find_package(Vulkan REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC spdlog::spdlog_header_only fmt::fmt-header-only imgui::imgui glm::glm-header-only EnTT::EnTT)

target_link_libraries(${PROJECT_NAME} PUBLIC glfw glad::glad Vulkan::Headers GPUOpen::VulkanMemoryAllocator)

# stb (image)
find_package(Stb REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${Stb_INCLUDE_DIR})
