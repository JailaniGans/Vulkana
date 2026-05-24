#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <vector>

class Context;
class Window;

/**
 * Swapchain - Mengelola swapchain, surface, depth, dan framebuffer.
 * Dua fase inisialisasi:
 *   1. buatSurfaceSwapchain() - surface + swapchain + image views + depth
 *   2. buatFramebuffers(renderPass) - framebuffer setelah render pass jadi
 * Ini memastikan format render pass cocok dengan format swapchain.
 */
class Swapchain {
public:
    Swapchain();
    ~Swapchain();

    bool buatSurfaceSwapchain(Context* context, Window* window);
    bool buatFramebuffers(VkRenderPass renderPass);
    void destroy();
    bool recreate(VkRenderPass renderPass);

    VkSwapchainKHR     getSwapchain()    const { return m_swapchain; }
    VkSurfaceKHR       getSurface()      const { return m_surface; }
    VkExtent2D         getExtent()       const { return m_extent; }
    VkFormat           getImageFormat()  const { return m_imageFormat; }
    uint32_t           getImageCount()   const { return static_cast<uint32_t>(m_images.size()); }
    const std::vector<VkImageView>&    getImageViews()    const { return m_imageViews; }
    const std::vector<VkFramebuffer>&  getFramebuffers()  const { return m_framebuffers; }

private:
    Context*           m_context;
    Window*            m_window;
    VkSurfaceKHR       m_surface;
    VkSwapchainKHR     m_swapchain;
    VkFormat           m_imageFormat;
    VkExtent2D         m_extent;
    std::vector<VkImage>       m_images;
    std::vector<VkImageView>   m_imageViews;
    std::vector<VkFramebuffer> m_framebuffers;
    VkImage       m_depthImage;
    VmaAllocation m_depthAllocation;
    VkImageView   m_depthImageView;
    VkRenderPass  m_renderPass;

    VkSurfaceFormatKHR pilihFormatSurface();
    VkPresentModeKHR pilihModePresent();
    VkExtent2D pilihExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    bool buatDepthResources();
};
