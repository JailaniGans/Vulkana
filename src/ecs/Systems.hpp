#pragma once

// Systems - fungsi global untuk update & render ECS

namespace Vulkana {

class Systems {
public:
    static void updateTransforms(float dt);
    static void renderMeshes();
};

}
