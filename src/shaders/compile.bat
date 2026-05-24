@echo off
glslc scene.vert -o ..\..\build\scene.vert.spv
glslc scene.frag -o ..\..\build\scene.frag.spv
echo Shaders compiled.
