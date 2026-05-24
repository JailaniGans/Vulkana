cmake_minimum_required(VERSION 3.10.0)
project(Vulkana VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------------------------------------------------------------------
# GLFW — build dari source via ext/glfw submodule
# -------------------------------------------------------------------
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS   OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS    OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL       OFF CACHE BOOL "" FORCE)
add_subdirectory(ext/glfw)

# -------------------------------------------------------------------
# Executable target
# -------------------------------------------------------------------
add_executable(Vulkana
    src/main.cpp

    # core/
    src/core/Application.cpp
    src/core/Engine.cpp
    src/core/Window.cpp
    src/core/Input.cpp
    src/core/Log.cpp

    # ecs/
    src/ecs/Systems.cpp

    # renderer/
    src/renderer/Context.cpp
    src/renderer/Swapchain.cpp
    src/renderer/Pipeline.cpp
    src/renderer/Buffer.cpp
    src/renderer/Descriptor.cpp
    src/renderer/Renderer.cpp
    src/renderer/VMA.cpp

    # Volk loader
    ext/Volk/volk.c
)

# -------------------------------------------------------------------
# Include directories (header-only dependencies)
# -------------------------------------------------------------------
target_include_directories(Vulkana PRIVATE
    src
    ext/entt/single_include
    ext/glm
    ext/VMA/include
    ext/Volk
    ext/Vulkan-Headers/include
)

# -------------------------------------------------------------------
# Platform-specific Vulkan defines
# -------------------------------------------------------------------
if(WIN32)
    target_compile_definitions(Vulkana PRIVATE VK_USE_PLATFORM_WIN32_KHR)
elseif(APPLE)
    target_compile_definitions(Vulkana PRIVATE VK_USE_PLATFORM_MACOS_MVK)
else()
    target_compile_definitions(Vulkana PRIVATE VK_USE_PLATFORM_XLIB_KHR)
endif()

# -------------------------------------------------------------------
# Shader directory (path absolut untuk runtime)
# -------------------------------------------------------------------
string(REPLACE "\\" "/" SHADER_DIR "${CMAKE_SOURCE_DIR}/src/shaders")
target_compile_definitions(Vulkana PRIVATE SHADER_DIR="${SHADER_DIR}")

# -------------------------------------------------------------------
# Link libraries
# -------------------------------------------------------------------
target_link_libraries(Vulkana PRIVATE
    glfw
)

# -------------------------------------------------------------------
# Shader compilation (opsional)
#   Jika glslc ada -> kompilasi .vert/.frag → .spv
#   Jika tidak   -> pakai .spv pre-compiled (tracked di git)
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
        file(SIZE "${SRC}" SIZE_VAL)
        if(SIZE_VAL GREATER 10)
            add_custom_command(OUTPUT "${SPV}"
                COMMAND ${GLSLC_EXECUTABLE} "${SRC}" -o "${SPV}"
                DEPENDS "${SRC}"
                COMMENT "Kompilasi ${F}"
            )
            list(APPEND SPV_FILES "${SPV}")
        endif()
    endforeach()
    if(SPV_FILES)
        add_custom_target(shaders ALL DEPENDS ${SPV_FILES})
        add_dependencies(Vulkana shaders)
        message(STATUS "glslc ditemukan — shader dikompilasi otomatis")
    endif()
else()
    message(STATUS "glslc tidak ditemukan — pakai .spv pre-compiled")
endif()
