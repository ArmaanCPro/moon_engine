cmake_minimum_required(VERSION 3.28)

project(engine CXX)

if (MSVC)
    add_compile_options(/MP)    #Use multiple processors when building
    add_compile_options(/W4 /wd4201 /WX) #Warning level 4, all warnings are errors
else ()
    add_compile_options(-W -Wall -Werror) #All Warnings, all warnings are errors
endif ()

set(SOURCES
        "src/test.h"
        "src/core/log.h"
        "src/core/log.cpp"
)

source_group("src" FILES ${SOURCES})

add_library(engine STATIC ${SOURCES})

target_include_directories(engine PUBLIC "${PROJECT_SOURCE_DIR}/src")
target_include_directories(engine PUBLIC "${PROJECT_SOURCE_DIR}/resources")

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(engine PUBLIC glfw glad::glad spdlog::spdlog_header_only)
