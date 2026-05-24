#pragma once

#include <glm/glm.hpp>

/**
 * TransformComponent - Posisi, rotasi, dan skala entity.
 */
struct TransformComponent {
    glm::vec3 posisi{0.0f, 0.0f, 0.0f};
    glm::vec3 rotasi{0.0f, 0.0f, 0.0f};
    glm::vec3 skala{1.0f, 1.0f, 1.0f};
};

/**
 * MeshComponent - ID mesh yang dirender oleh entity.
 */
struct MeshComponent {
    uint32_t idMesh = 0;
};
