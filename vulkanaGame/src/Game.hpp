// ===================================================================
// Game Class
// ===================================================================
// Main game application class inheriting from Vulkana::Application.
// Manages gameplay logic, scene states, and game systems.
// ===================================================================

#pragma once

#include "core/Application.hpp"
#include "ecs/Components.hpp"
#include <entt/entt.hpp>

class Game : public Vulkana::Application
{
public:
    Game() = default;
    ~Game() override = default;

    // Framework hooks
    void onInit() override;
    void onUpdate(float dt) override;
    void onRender() override;
    void onCleanup() override;

private:
    entt::registry m_registry;
};
