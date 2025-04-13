cmake_minimum_required(VERSION 3.28)

project(moon_editor CXX)

if (MSVC)
    add_compile_options(/W4 /wd4201 /WX) #Warning level 4, all warnings are errors
else ()
    add_compile_options(-W -Wall -Werror) #All Warnings, all warnings are errors
endif ()

set(SOURCES
        "src/editor.cpp"
)

source_group("src" FILES ${SOURCES})

add_executable(${PROJECT_NAME} ${SOURCES})

#target_compile_definitions(moon_editor PRIVATE MOON_IS_MONOLITHIC)

target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/src")
target_link_libraries(${PROJECT_NAME} PRIVATE moon_engine)
