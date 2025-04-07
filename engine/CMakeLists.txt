cmake_minimum_required(VERSION 3.28)

project(moon_engine CXX)

if (MSVC)
    add_compile_options(/MP)    #Use multiple processors when building
    add_compile_options(/W4 /wd4201 /WX) #Warning level 4, all warnings are errors
else ()
    add_compile_options(-W -Wall -Werror) #All Warnings, all warnings are errors
endif ()

file(GLOB_RECURSE ENGINE_SOURCES "src/*.cpp")
file(GLOB_RECURSE ENGINE_HEADERS "include/*.h" "src/*.h")

# Create a shared library instead of a static one
add_library(moon_engine SHARED ${ENGINE_SOURCES} ${ENGINE_HEADERS} )

# Define the MOON_ENGINE_EXPORTS macro when building the moon_engine
target_compile_definitions(moon_engine PRIVATE MOON_ENGINE_EXPORTS)

# Set include directories
target_include_directories(moon_engine
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_precompile_headers(moon_engine PRIVATE src/moonpch.h)


find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(moon_engine PUBLIC glfw glad::glad spdlog::spdlog_header_only)
