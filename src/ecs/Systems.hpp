#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <vector>

class Window;
class Renderer;
class Pipeline;
class Buffer;
class Descriptor;

/**
 * UniformBufferObject - Data UBO untuk shader vertex.
 * Dikirim ke GPU tiap frame via uniform buffer.
 */
struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

/**
 * RenderSystem - Sistem rendering utama.
 * Memiliki pipeline, vertex buffer, UBO, dan descriptor.
 * Memanggil Renderer::beginFrame()/endFrame() tiap frame
 * dan mencatat perintah draw segitiga.
 */
class RenderSystem {
public:
    RenderSystem();
    ~RenderSystem();

    bool init(Window* window);
    void update(entt::registry& registry);
    void waitIdle();

private:
    Renderer*    m_renderer;
    Pipeline*    m_pipeline;
    Descriptor*  m_descriptor;
    Buffer*      m_vertexBuffer;
    std::vector<Buffer*> m_uniformBuffers;

    float m_waktuBerjalan;

    bool buatVertexBuffer();
    bool buatUniformBuffers();
    bool buatPipeline();
};

/**
 * PhysicsSystem - Sistem fisika (placeholder).
 * Memperbarui komponen transform tiap frame.
 */
class PhysicsSystem {
public:
    PhysicsSystem();
    ~PhysicsSystem();

    void update(entt::registry& registry, float deltaTime);

private:
    glm::vec3 m_gravitas;
};
