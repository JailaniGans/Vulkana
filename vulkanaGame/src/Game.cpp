// ===================================================================
// Game Implementation
// ===================================================================

#include "Game.hpp"
#include "core/Log.hpp"
#include "core/Input.hpp"
#include <GLFW/glfw3.h>

void Game::onInit()
{
    LOG_INFO("Game initialized");
    // Initialize game systems, load assets, setup scenes
}

void Game::onUpdate(float dt)
{
    // Update game logic
    if (Vulkana::Input::isKeyPressed(GLFW_KEY_ESCAPE))
    {
        LOG_INFO("ESC pressed - shutting down");
    }
}

void Game::onRender()
{
    // Render game state
    // Renderer is data-driven and handles primitives/matrices
}

void Game::onCleanup()
{
    LOG_INFO("Game shutting down");
    // Cleanup game resources
}
