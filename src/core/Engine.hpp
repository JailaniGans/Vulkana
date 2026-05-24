#pragma once

// Engine - loop utama: inisialisasi, update, render, cleanup
//   Hubungkan Window + Context + Swapchain + Renderer

#include <memory>
#include <volk.h>

namespace Vulkana {

class Window;
class Context;
class Swapchain;
class Renderer;
class Application;

class Engine {
public:
    Engine(Application& app);
    ~Engine();

    void run();

private:
    void init();
    void cleanup();
    void recreateSwapchain();

    Application& m_app;
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Context> m_context;
    std::unique_ptr<Swapchain> m_swapchain;
    std::unique_ptr<Renderer> m_renderer;

    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    float m_lastTime = 0.0f;
};

}
