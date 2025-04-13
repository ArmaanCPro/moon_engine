cmake_minimum_required(VERSION 3.28)

# Set vcpkg linkage types (BEFORE toolchain file)
set(VCPKG_CRT_LINKAGE "dynamic" CACHE STRING "")
set(VCPKG_LIBRARY_LINKAGE "static" CACHE STRING "")

# Additional package-specific settings (optional)
set(GLAD_PROFILE "core" CACHE STRING "")

# MINGW keyword doesn't always work. However, selecting the preset usually works. VCPKG_CM... is defined by preset
if (MINGW OR VCPKG_CMAKE_SYSTEM STREQUAL MINGW)
    set(VCPKG_TARGET_TRIPLET "x64-mingw-static" CACHE STRING "")
    set(VCPKG_HOST_TRIPLET "x64-mingw-static" CACHE STRING "")
endif()

# enforce static linking in msvc. not sure if this is needed
if (MSVC OR VCPKG_CMAKE_SYSTEM STREQUAL MSVC)
    set(VCPKG_TARGET_TRIPLET "x64-windows-static-md" CACHE STRING "")
    set(VCPKG_HOST_TRIPLET "x64-windows-static-md" CACHE STRING "")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    add_compile_options(
            /utf-8
            /wd4251
            /EHsc
    )
endif()

# vcpkg
if (DEFINED ENV{VCPKG_ROOT})
    set(VCPKG_ROOT $ENV{VCPKG_ROOT})
else ()
    message(STATUS "Downloading vcpkg")
    include(FetchContent)
    FetchContent_Declare(vcpkg
            GIT_REPOSITORY https://github.com/microsoft/vcpkg/
    )
    FetchContent_MakeAvailable(vcpkg)
    set(VCPKG_ROOT ${vcpkg_SOURCE_DIR})
endif()
message(STATUS "Using VCPKG_ROOT: ${VCPKG_ROOT}")
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "vcpkg toolchain file")

#--------
# project
# -------
project(moon CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG ME_DEBUG)
else ()
    add_compile_definitions(NDEBUG ME_NDEBUG)
endif ()

# platform specific stuff
if (WIN32 OR MINGW)
    set(PLATFORM_DEFINITIONS ME_WINDOWS)
elseif (APPLE)
    set(PLATFORM_DEFINITIONS ME_OSX)
elseif (UNIX AND NOT APPLE)
    set(PLATFORM_DEFINITIONS ME_LINUX)
endif ()
add_compile_definitions(${PLATFORM_DEFINITIONS})

# multiprocessor compiling
if(MSVC)
    add_compile_options(/MP)
else()
    include(ProcessorCount)
    ProcessorCount(N)
    if(NOT N EQUAL 0)
        set(CMAKE_BUILD_PARALLEL_LEVEL ${N})
    endif()
endif()

add_subdirectory(engine)
add_subdirectory(editor)
add_subdirectory(sandbox)
