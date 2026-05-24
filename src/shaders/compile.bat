@echo off
rem Kompilasi shader GLSL ke SPIR-V (butuh glslc dari Vulkan SDK)
glslc scene.vert -o scene.vert.spv || echo Gagal kompilasi scene.vert
glslc scene.frag -o scene.frag.spv || echo Gagal kompilasi scene.frag
echo Selesai
