@echo off
REM compile.bat - Kompilasi GLSL ke SPIR-V untuk Vulkana.
REM SPIR-V output di build/ (working directory root project).

set VK_SDK_PATH=C:\VulkanSDK\1.4.350.0

echo Kompilasi scene.vert...
"%VK_SDK_PATH%\Bin\glslc.exe" scene.vert -o ..\..\build\scene.vert.spv
if %ERRORLEVEL% neq 0 (
    echo Gagal kompilasi vertex shader
    exit /b %ERRORLEVEL%
)

echo Kompilasi scene.frag...
"%VK_SDK_PATH%\Bin\glslc.exe" scene.frag -o ..\..\build\scene.frag.spv
if %ERRORLEVEL% neq 0 (
    echo Gagal kompilasi fragment shader
    exit /b %ERRORLEVEL%
)

echo Semua shader berhasil dikompilasi.
