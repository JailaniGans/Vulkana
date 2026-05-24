#include "ecs/Systems.hpp"
#include "ecs/Components.hpp"
#include "renderer/Renderer.hpp"
#include "renderer/Pipeline.hpp"
#include "renderer/Buffer.hpp"
#include "renderer/Descriptor.hpp"
#include "renderer/Context.hpp"
#include "renderer/Swapchain.hpp"
#include "core/Log.hpp"
#include "core/Window.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>

RenderSystem::RenderSystem()
    : m_renderer(nullptr)
    , m_pipeline(nullptr)
    , m_descriptor(nullptr)
    , m_vertexBuffer(nullptr)
    , m_waktuBerjalan(0.0f)
{
}

RenderSystem::~RenderSystem() {
    waitIdle();
    for (auto buf : m_uniformBuffers) delete buf;
    m_uniformBuffers.clear();
    delete m_vertexBuffer;
    delete m_pipeline;
    delete m_descriptor;
    delete m_renderer;
}

bool RenderSystem::init(Window* window) {
    Log::info("RenderSystem menginisialisasi...");

    m_renderer = new Renderer();
    if (!m_renderer->init(window)) {
        Log::error("RenderSystem: Gagal inisialisasi Renderer");
        return false;
    }

    m_descriptor = new Descriptor();
    if (!m_descriptor->create(m_renderer->getContext(), Renderer::MAX_FRAMES_IN_FLIGHT)) {
        Log::error("RenderSystem: Gagal inisialisasi Descriptor");
        return false;
    }

    if (!buatVertexBuffer()) return false;
    if (!buatUniformBuffers()) return false;

    if (!m_descriptor->allocateSets(m_uniformBuffers)) {
        Log::error("RenderSystem: Gagal alokasi descriptor set");
        return false;
    }

    if (!buatPipeline()) return false;

    Log::info("RenderSystem berhasil diinisialisasi");
    return true;
}

void RenderSystem::update(entt::registry& registry) {
    (void)registry;

    // Update waktu berjalan untuk animasi
    m_waktuBerjalan += 0.01f;

    // Update UBO dengan rotasi segitiga
    uint32_t frameSekarang = m_renderer->getCurrentFrame();
    UniformBufferObject ubo{};

    ubo.model = glm::rotate(glm::mat4(1.0f), m_waktuBerjalan * glm::radians(45.0f),
                            glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
                           glm::vec3(0.0f, 0.0f, 0.0f),
                           glm::vec3(0.0f, 0.0f, 1.0f));
    VkExtent2D extent = m_renderer->getSwapchain()->getExtent();
    float rasio = extent.width / static_cast<float>(extent.height);
    ubo.proj = glm::perspective(glm::radians(45.0f), rasio, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1.0f; // Vulkan NDC: Y terbalik

    m_uniformBuffers[frameSekarang]->upload(&ubo, sizeof(ubo));

    // Frame rendering
    uint32_t indeksGambar = 0;
    VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;

    if (!m_renderer->beginFrame(indeksGambar, cmdBuffer)) {
        return; // swapchain sedang di-recreate
    }

    // Bind pipeline, vertex buffer, descriptor set, lalu draw
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->getPipeline());

    VkDeviceSize offset = 0;
    VkBuffer bufferVertex = m_vertexBuffer->getBuffer();
    vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &bufferVertex, &offset);

    VkDescriptorSet descriptorSet = m_descriptor->getSet(frameSekarang);
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_pipeline->getLayout(), 0, 1,
                            &descriptorSet, 0, nullptr);

    vkCmdDraw(cmdBuffer, 3, 1, 0, 0);

    m_renderer->endFrame(indeksGambar, cmdBuffer);
}

void RenderSystem::waitIdle() {
    if (m_renderer) m_renderer->waitIdle();
}

bool RenderSystem::buatVertexBuffer() {
    // Data tiga vertex segitiga: posisi (vec3) + warna (vec3)
    struct Vertex { glm::vec3 posisi; glm::vec3 warna; };

    const std::array<Vertex, 3> dataVertex = {{
        {{ 0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}}, // merah di bawah
        {{ 0.5f,  0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // hijau di kanan atas
        {{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}  // biru di kiri atas
    }};
    VkDeviceSize ukuran = sizeof(Vertex) * dataVertex.size();

    m_vertexBuffer = new Buffer();
    if (!m_vertexBuffer->create(m_renderer->getContext(), ukuran,
                                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                VMA_MEMORY_USAGE_AUTO_PREFER_HOST)) {
        Log::error("Gagal membuat vertex buffer");
        return false;
    }
    m_vertexBuffer->upload(dataVertex.data(), ukuran);

    Log::info("Vertex buffer segitiga dibuat");
    return true;
}

bool RenderSystem::buatUniformBuffers() {
    VkDeviceSize ukuranUBO = sizeof(UniformBufferObject);
    m_uniformBuffers.resize(Renderer::MAX_FRAMES_IN_FLIGHT);

    for (uint32_t i = 0; i < Renderer::MAX_FRAMES_IN_FLIGHT; i++) {
        m_uniformBuffers[i] = new Buffer();
        if (!m_uniformBuffers[i]->create(m_renderer->getContext(), ukuranUBO,
                                          VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                          VMA_MEMORY_USAGE_AUTO_PREFER_HOST)) {
            Log::error("Gagal membuat uniform buffer");
            return false;
        }
    }

    Log::info("Uniform buffer dibuat");
    return true;
}

bool RenderSystem::buatPipeline() {
    VkExtent2D extent = m_renderer->getSwapchain()->getExtent();
    m_pipeline = new Pipeline();
    if (!m_pipeline->create(m_renderer->getContext(), extent,
                            m_renderer->getRenderPass(),
                            m_descriptor->getLayout())) {
        Log::error("Gagal membuat pipeline grafis");
        return false;
    }
    return true;
}

// --- PhysicsSystem ---

PhysicsSystem::PhysicsSystem()
    : m_gravitas(0.0f, -9.81f, 0.0f)
{
}

PhysicsSystem::~PhysicsSystem() {
}

void PhysicsSystem::update(entt::registry& registry, float deltaTime) {
    (void)registry;
    (void)deltaTime;
    // TODO: update fisika entity
}
