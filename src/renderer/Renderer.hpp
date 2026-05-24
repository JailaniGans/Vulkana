#pragma once

// Renderer - command pool, command buffer, sinkronisasi, draw & present

#include <volk.h>
#include <vector>
#include <array>
#include <cstdint>

namespace Vulkana {

class Context;
class Swapchain;

class Renderer {
public:
    Renderer() = default;
    ~Renderer();

    void init(Context& context, Swapchain& swapchain);
    void cleanup();

    bool beginFrame();
    bool endFrame();
    void drawIndexed(uint32_t indexCount, uint32_t instanceCount = 1);

    VkCommandBuffer currentCmd() const { return m_cmdBufs[m_currentImage]; }

private:
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();

    VkDevice m_device = VK_NULL_HANDLE;
    Context* m_context = nullptr;
    Swapchain* m_swapchain = nullptr;

    VkCommandPool m_cmdPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> m_cmdBufs;
    uint32_t m_currentImage = 0;

    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
    std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> m_imageAvailable{};
    std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> m_renderFinished{};
    std::array<VkFence, MAX_FRAMES_IN_FLIGHT> m_inFlightFence{};
    uint32_t m_frameIndex = 0;
};

}
