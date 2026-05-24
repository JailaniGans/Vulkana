#pragma once

// Swapchain - buffer berganda + render pass + framebuffer

#include <volk.h>
#include <vector>
#include <cstdint>

namespace Vulkana {

struct SwapchainImage
{
    VkImage image = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
};

class Swapchain {
public:
    Swapchain() = default;
    ~Swapchain();

    void init(VkPhysicalDevice gpu, VkDevice device, VkSurfaceKHR surface,
              int width, int height);
    void cleanup();
    void recreate(int width, int height);

    VkSwapchainKHR handle() const { return m_swapchain; }
    const std::vector<SwapchainImage>& images() const { return m_images; }
    VkFormat format() const { return m_format; }
    VkExtent2D extent() const { return m_extent; }
    VkRenderPass renderPass() const { return m_renderPass; }
    const std::vector<VkFramebuffer>& framebuffers() const { return m_framebuffers; }
    VkImageView depthView() const { return m_depthView; }

private:
    void createSwapchain(int w, int h);
    void createImageViews();
    void createDepthResources();
    void createRenderPass();
    void createFramebuffers();

    VkDevice m_device = VK_NULL_HANDLE;
    VkPhysicalDevice m_gpu = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;

    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<SwapchainImage> m_images;
    VkFormat m_format = VK_FORMAT_UNDEFINED;
    VkExtent2D m_extent{};

    VkImage m_depthImage = VK_NULL_HANDLE;
    VkDeviceMemory m_depthMemory = VK_NULL_HANDLE;
    VkImageView m_depthView = VK_NULL_HANDLE;

    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_framebuffers;
};

}
