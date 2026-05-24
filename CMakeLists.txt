cmake_minimum_required(VERSION 3.10.0)
project(Vulkana VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(VULKAN_SDK_PATH "$ENV{VULKAN_SDK}")
set(GLFW_PATH "C:/glfw-3.4.bin.WIN64/glfw-3.4.bin.WIN64")

add_executable(Vulkana
    src/main.cpp
    src/core/Application.cpp
    src/core/Engine.cpp
    src/core/Window.cpp
    src/core/Input.cpp
    src/core/Log.cpp
    src/renderer/Context.cpp
    src/renderer/Swapchain.cpp
    src/renderer/Pipeline.cpp
    src/renderer/Buffer.cpp
    src/renderer/Descriptor.cpp
    src/renderer/Renderer.cpp
    src/renderer/VMA.cpp
    src/ecs/Systems.cpp
)

target_include_directories(Vulkana PRIVATE
    src
    ext/entt/single_include
    ext/VMA/include
    ext/glm
    ${GLFW_PATH}/include
    ${VULKAN_SDK_PATH}/Include
)

target_compile_definitions(Vulkana PRIVATE
    VK_USE_PLATFORM_WIN32_KHR
)

target_link_directories(Vulkana PRIVATE
    ${GLFW_PATH}/lib-vc2022
    ${VULKAN_SDK_PATH}/Lib
)

target_link_libraries(Vulkana PRIVATE
    glfw3
    vulkan-1
)
