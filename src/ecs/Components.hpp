#pragma once

// Components - komponen ECS untuk entitas game

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdint>
#include <vector>

namespace Vulkana {

struct Transform
{
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f};
    glm::vec3 scale{1.0f};

    glm::mat4 matrix() const
    {
        glm::mat4 m = glm::translate(glm::mat4(1.0f), position);
        m = glm::rotate(m, rotation.x, glm::vec3(1, 0, 0));
        m = glm::rotate(m, rotation.y, glm::vec3(0, 1, 0));
        m = glm::rotate(m, rotation.z, glm::vec3(0, 0, 1));
        m = glm::scale(m, scale);
        return m;
    }
};

struct Mesh
{
    std::vector<glm::vec3> positions;
    std::vector<uint32_t> indices;
};

struct Camera
{
    glm::mat4 projection{1.0f};
    glm::mat4 view{1.0f};

    void setPerspective(float fov, float aspect, float nearZ, float farZ)
    {
        projection = glm::perspective(fov, aspect, nearZ, farZ);
    }

    void setLookAt(glm::vec3 eye, glm::vec3 target, glm::vec3 up)
    {
        view = glm::lookAt(eye, target, up);
    }
};

}
