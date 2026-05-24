#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class Context;
class Window;
class Swapchain;

/**
 * Renderer - Orkestrator rendering Vulkan.
 *
 * Sinkronisasi:
 *   - Fence per frame (MAX_FRAMES_IN_FLIGHT=2) untuk CPU-GPU sync
 *   - Semaphore render-selesai per swapchain image, di-index dengan imageIndex
 *   - vkAcquireNextImageKHR pakai fence (bukan semaphore) untuk menghindari
 *     konflik reuse semaphore (VUID-vkQueueSubmit-pSignalSemaphores-00067)
 *
 * Pipeline, vertex buffer, descriptor set dibuat oleh sistem pengguna
 * (misal RenderSystem) dan di-binding di antara beginFrame()/endFrame().
 */
class Renderer {
public:
    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    Renderer();
    ~Renderer();

    bool init(Window* window);
    void cleanup();

    bool beginFrame(uint32_t& imageIndex, VkCommandBuffer& cmdBuffer);
    void endFrame(uint32_t imageIndex, VkCommandBuffer cmdBuffer);

    void waitIdle();

    Context*   getContext()     const { return m_context; }
    Swapchain* getSwapchain()   const { return m_swapchain; }
    VkRenderPass getRenderPass() const { return m_renderPass; }
    VkCommandPool getCommandPool() const { return m_commandPool; }
    uint32_t   getCurrentFrame() const { return m_currentFrame; }

private:
    Context*   m_context;
    Swapchain* m_swapchain;
    Window*    m_window;

    VkRenderPass           m_renderPass;
    VkCommandPool          m_commandPool;
    std::vector<VkCommandBuffer> m_commandBuffers;

    // Semaphore per swapchain image — indexed oleh imageIndex
    std::vector<VkSemaphore> m_semaphoreRenderSelesai;
    // Fence per frame in-flight
    std::vector<VkFence>     m_fenceDalamProses;

    uint32_t m_currentFrame;

    bool buatRenderPass(VkFormat formatGambar);
    bool buatCommandPool();
    bool buatCommandBuffers();
    bool buatSyncObjects(uint32_t jumlahGambar);
};
