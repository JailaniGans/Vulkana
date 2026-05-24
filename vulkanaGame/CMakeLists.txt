# ===================================================================
# VULKANA GAME CLIENT EXECUTABLE
# ===================================================================
# Game-specific logic, components, systems, and scenes.
# Inherits from Vulkana::Application (core framework).
# ===================================================================

# Game executable target
add_executable(VulkanaGame
    main.cpp
    src/Game.cpp
    src/components/PlayerComponent.hpp
    src/systems/PlayerSystem.hpp
    src/scenes/MainScene.hpp
)

# Link to core framework library
target_link_libraries(VulkanaGame PRIVATE Vulkana-Core)

# Include directories
target_include_directories(VulkanaGame PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/ext/entt/single_include
    ${CMAKE_SOURCE_DIR}/ext/glfw/include
    ${CMAKE_SOURCE_DIR}/ext/glm
    ${CMAKE_SOURCE_DIR}/ext/VMA/include
    ${CMAKE_SOURCE_DIR}/ext/Volk
    ${CMAKE_SOURCE_DIR}/ext/Vulkan-Headers/include
)

message(STATUS "Game executable: VulkanaGame")
