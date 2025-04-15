cmake_minimum_required(VERSION 3.28)

project(moon_editor CXX)

if (MSVC)
    add_compile_options(/W4 /wd4201 /wd4100 /WX) #Warning level 4, all warnings are errors
else ()
    add_compile_options(-W -Wall -Werror -Wno-unused-parameter) #All Warnings, all warnings are errors
endif ()

set(SOURCES
        "src/editor.cpp"
)

source_group("src" FILES ${SOURCES})

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/src")
target_link_libraries(${PROJECT_NAME} PRIVATE moon_engine)

if (NOT IS_MONOLITHIC)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:moon_engine>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif ()
