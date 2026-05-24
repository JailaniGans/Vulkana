#include "ecs/Systems.hpp"
#include "ecs/Components.hpp"
#include "core/Log.hpp"

namespace Vulkana {

void Systems::updateTransforms(float)
{
    // TODO: iterasi entitas dengan komponen Transform
    LOG_INFO("Systems::updateTransforms dipanggil");
}

void Systems::renderMeshes()
{
    // TODO: iterasi entitas dengan komponen Mesh & Transform, kirim ke GPU
    LOG_INFO("Systems::renderMeshes dipanggil");
}

}
