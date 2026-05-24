#pragma once

struct TransformComponent {
    float x, y, z;
};

struct MeshComponent {
    unsigned int vertexBuffer;
    unsigned int indexBuffer;
};
