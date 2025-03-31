cmake_minimum_required(VERSION 3.28)

project(editor CXX)

if (MSVC)
    add_compile_options(/MP)    #Use multiple processors when building
    add_compile_options(/W4 /wd4201 /WX) #Warning level 4, all warnings are errors
else ()
    add_compile_options(-W -Wall -Werror) #All Warnings, all warnings are errors
endif ()

set(SOURCES
        "src/test.cpp"
)

source_group("src" FILES ${SOURCES})

add_executable(editor ${SOURCES})

target_include_directories(editor PRIVATE "${PROJECT_SOURCE_DIR}/src")
target_link_libraries(editor PRIVATE engine)
