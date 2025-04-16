# Build

## Table of Contents

- [Windows](#windows)
- [Linux](#linux)
- [macOS](#macos)
- [Flags](#cmake-configure-flags)

## Windows

Using Visual Studio IDE
- You can either directly open the cmake project in Visual Studio by selecting the "Open a local folder" option, or:
- In a terminal, run:
    - `cmake --preset debug-msvc -G "Visual Studio 17 2022"`
    - Then you can open a file explorer in the `build/debug-msvc` directory
    - Then open the .sln file, and you should be set!
   

Using MSVC without the Visual Studio IDE (i.e., Clion, VS Code)
- Make sure you have Ninja installed through something like chocolatey or winget
- In a terminal, run:
    - `cmake --preset debug-msvc`
    - `cd build/debug-msvc`
    - `cmake --build .` (Optionally append: `--target editor` or some other target)
- Or you can select the debug/release-msvc preset in your ide
   

Using MinGW
- Make sure basic MinGW packages are installed like libc++ and gcc
- In a terminal, run:
    - `cmake --preset debug-mingw` (or `release-mingw`)
- Or select the desired preset in your ide

## Linux
I seldom test Linux on PopOS and Ubuntu (WSL), but note that I regularly use Windows and therefore windows is the most supported platform
1. Makefiles
    - Just add the `-G "Unix Makefiles"` command in all the following config command, otherwise Ninja is the default generator
   
2. Default Configuration
    - The `debug-default` and `release default` commands should suffice
    - Run:
        - `cmake --preset debug-default`
        - `cd build/debug-default`
        - `cmake --build .` (Optionally append: `--target editor` or some other target)

## MacOS

MacOS <i>should</i> have no problems and <i>should</i> follow the same config as Linux, but who knows. I have not tested MacOS builds.

## CMake configure flags

Example usage: `cmake --preset debug-default -DIS_MONLITHIC=ON`

### All platforms
| Flag          |   | Description                                                                      | Default | Note                                                           |
|---------------|:--|----------------------------------------------------------------------------------|---------|----------------------------------------------------------------|
| IS_MONOLITHIC |   | Allows compiling moon_engine as a static library and linking projects statically | OFF     | May require deleting the CMakeCache file in the build location |
