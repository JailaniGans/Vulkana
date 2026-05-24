// ===================================================================
// Player Component
// ===================================================================
// Game-specific component for player entities.
// ===================================================================

#pragma once

#include <glm/glm.hpp>

struct PlayerComponent
{
    float health = 100.0f;
    float speed = 5.0f;
    glm::vec3 velocity{0.0f};
};
