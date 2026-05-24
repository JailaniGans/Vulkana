cmake_minimum_required(VERSION 3.10.0)
project(Vulkana VERSION 0.1.0 LANGUAGES C CXX)

# ===================================================================
# PROJECT ARCHITECTURE
# ===================================================================
# Vulkana is a modular Game Framework/Engine with strict separation:
# 
# src/         → VULKANA CORE LIBRARY (Vulkana namespace)
#   Agnostic to gameplay logic. Handles engine lifecycle, raw Vulkan
#   rendering abstractions, windowing, input tracking, generic ECS utilities.
#
# vulkanaGame/ → GAME EXECUTABLE/CLIENT (global/game-specific namespaces)
#   Concrete gameplay logic, game-specific components (Health, AI),
#   gameplay systems (Movement, Collision), and scene states.
#
# ===================================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------------------------------------------------------------------
# EXTERNAL DEPENDENCIES (ext/)
# -------------------------------------------------------------------
# Build GLFW from source
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS   OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS    OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL       OFF CACHE BOOL "" FORCE)
add_subdirectory(ext/glfw)

# Header-only: entt, glm, VMA, Volk, Vulkan-Headers

# -------------------------------------------------------------------
# VULKANA CORE LIBRARY (src/)
# ===================================================================
# Core Framework: Application lifecycle, Vulkan abstractions, 
# windowing, input, ECS utilities — NO game-specific logic
# ===================================================================
# -------------------------------------------------------------------
add_library(Vulkana-Core STATIC
    # core/ — Engine lifecycle and subsystem management
    src/core/Application.cpp
    src/core/Engine.cpp
    src/core/Window.cpp
    src/core/Input.cpp
    src/core/Log.cpp

    # gameplay/ — Actor/Component framework
    # (headers only for now)

    # ecs/ — Generic ECS registry wrapper
    src/ecs/Systems.cpp

    # renderer/ — Raw Vulkan abstractions (data-driven)
    src/renderer/Context.cpp
    src/renderer/Swapchain.cpp
    src/renderer/Pipeline.cpp
    src/renderer/Buffer.cpp
    src/renderer/Descriptor.cpp
    src/renderer/Renderer.cpp
    src/renderer/VMA.cpp

    # networking/ — Client-server network layer
    src/networking/NetworkDriver.cpp
    src/networking/NetworkSystem.cpp

    # scripting/ — Embedded scripting VM abstraction
    src/scripting/ScriptingSubsystem.cpp

    # Volk loader for dynamic Vulkan
    ext/Volk/volk.c
)

# Core library public headers
target_include_directories(Vulkana-Core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/entt/single_include
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/glm
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/VMA/include
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/Volk
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/Vulkan-Headers/include
)

# Platform-specific Vulkan defines
if(WIN32)
    target_compile_definitions(Vulkana-Core PRIVATE VK_USE_PLATFORM_WIN32_KHR)
elseif(APPLE)
    target_compile_definitions(Vulkana-Core PRIVATE VK_USE_PLATFORM_MACOS_MVK)
else()
    target_compile_definitions(Vulkana-Core PRIVATE VK_USE_PLATFORM_XLIB_KHR)
endif()

# Shader directory (absolute path for runtime)
string(REPLACE "\\" "/" SHADER_DIR "${CMAKE_SOURCE_DIR}/src/shaders")
target_compile_definitions(Vulkana-Core PRIVATE SHADER_DIR="${SHADER_DIR}")

# Link core library to GLFW
target_link_libraries(Vulkana-Core PRIVATE glfw)

# Link WinSock2 on Windows for networking
if(WIN32)
    target_link_libraries(Vulkana-Core PRIVATE ws2_32)
endif()

# -------------------------------------------------------------------
# GAME EXECUTABLE (vulkanaGame/)
# ===================================================================
# Game Client: Gameplay logic, game-specific components/systems,
# scene states. Inherits from Vulkana::Application.
# ===================================================================
# -------------------------------------------------------------------
add_subdirectory(vulkanaGame)

# -------------------------------------------------------------------
# SHADER COMPILATION (Optional)
# -------------------------------------------------------------------
# If glslc is available → auto-compile .vert/.frag → .spv
# Otherwise → use pre-compiled .spv files (tracked in git)
# -------------------------------------------------------------------
find_program(GLSLC_EXECUTABLE glslc
    HINTS
        "$ENV{VULKAN_SDK}/Bin"
        "$ENV{VULKAN_SDK}/bin"
)

if(GLSLC_EXECUTABLE)
    set(SHADER_DIR "${CMAKE_SOURCE_DIR}/src/shaders")
    set(SPV_FILES)
    foreach(F scene.vert scene.frag)
        set(SRC "${SHADER_DIR}/${F}")
        set(SPV "${SHADER_DIR}/${F}.spv")
        if(EXISTS "${SRC}")
            add_custom_command(OUTPUT "${SPV}"
                COMMAND ${GLSLC_EXECUTABLE} "${SRC}" -o "${SPV}"
                DEPENDS "${SRC}"
                COMMENT "Compiling shader: ${F}"
            )
            list(APPEND SPV_FILES "${SPV}")
        endif()
    endforeach()
    if(SPV_FILES)
        add_custom_target(shaders ALL DEPENDS ${SPV_FILES})
        add_dependencies(VulkanaGame shaders)
        message(STATUS "glslc found — shaders will be auto-compiled")
    endif()
else()
    message(STATUS "glslc not found — using pre-compiled .spv files")
endif()

# -------------------------------------------------------------------
# SUMMARY
# -------------------------------------------------------------------
message(STATUS "=== Vulkana Configuration ===")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Core Library: Vulkana-Core (STATIC)")
message(STATUS "Executable: VulkanaGame")
message(STATUS "Shader Dir: ${SHADER_DIR}")
message(STATUS "=============================")

