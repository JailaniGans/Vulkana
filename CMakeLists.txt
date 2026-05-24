cmake_minimum_required(VERSION 3.10.0)
project(Vulkana VERSION 0.1.0 LANGUAGES C CXX)

set(VULKAN_SDK_PATH "$ENV{VULKAN_SDK}")
set(GLFW_PATH "C:/glfw-3.4.bin.WIN64/glfw-3.4.bin.WIN64")

add_executable(Vulkana
    src/main.cpp
    src/core/Application.cpp
    src/core/Engine.cpp
    src/core/Window.cpp
    src/core/Input.cpp
    src/core/Log.cpp
    src/ecs/Systems.cpp
    src/renderer/Context.cpp
    src/renderer/Swapchain.cpp
    src/renderer/Pipeline.cpp
    src/renderer/Buffer.cpp
    src/renderer/Descriptor.cpp
    src/renderer/Renderer.cpp
)

target_include_directories(Vulkana PRIVATE
    src
    ${GLFW_PATH}/include
    ${VULKAN_SDK_PATH}/Include
)

target_link_directories(Vulkana PRIVATE
    ${GLFW_PATH}/lib-vc2022
    ${VULKAN_SDK_PATH}/Lib
)

target_link_libraries(Vulkana PRIVATE
    glfw3
    vulkan-1
)
