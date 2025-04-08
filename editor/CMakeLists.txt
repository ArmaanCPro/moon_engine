cmake_minimum_required(VERSION 3.28)

project(moon_editor CXX)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    if (MSVC)
        add_compile_options(/W4 /wd4201 /WX) #Warning level 4, all warnings are errors
    else ()
        add_compile_options(-W -Wall -Werror) #All Warnings, all warnings are errors
    endif ()
endif()

set(SOURCES
        "src/editor.cpp"
)

source_group("src" FILES ${SOURCES})

add_executable(moon_editor ${SOURCES})

target_include_directories(moon_editor PRIVATE "${PROJECT_SOURCE_DIR}/src")
target_link_libraries(moon_editor PRIVATE moon_engine)

add_custom_command(TARGET moon_editor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:moon_engine>
        $<TARGET_FILE_DIR:moon_editor>
)
