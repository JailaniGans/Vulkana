// ===================================================================
// Main Scene
// ===================================================================
// Game-specific scene setup and state management.
// ===================================================================

#pragma once

#include <entt/entt.hpp>

class MainScene
{
public:
    static void setup(entt::registry& registry);
    static void update(entt::registry& registry, float dt);
};
