// ===================================================================
// Player System
// ===================================================================
// Game-specific system managing player logic.
// ===================================================================

#pragma once

#include <entt/entt.hpp>

class PlayerSystem
{
public:
    static void update(entt::registry& registry, float dt);
};
