@echo off
REM Compile texture.slang into Vulkan SPIR-V vertex and fragment modules

REM Path to your slangc.exe (remove quotes if already in PATH)
set SLANGC=slangc.exe

REM Common options:
REM -target spirv: Compile to SPIR-V binary
REM -profile vs_6_0 / ps_6_0: Shader Model profiles (vertex/fragment)
REM -entry: Which function in your .slang file to compile
REM -fvk-use-entrypoint-name: Ensure entrypoint name in SPIR-V matches source

%SLANGC% texture.slang -target spirv -profile vs_6_6 -entry vertexMain ^
    -fvk-use-entrypoint-name -o texture.vert.spirv

%SLANGC% texture.slang -target spirv -profile ps_6_6 -entry fragmentMain ^
    -fvk-use-entrypoint-name -o texture.frag.spirv

echo Done! Generated texture.vert.spirv and texture.frag.spirv
pause
