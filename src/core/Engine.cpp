#include "core/Engine.hpp"
#include "core/Window.hpp"
#include "core/Input.hpp"
#include "core/Application.hpp"
#include "renderer/Context.hpp"
#include "renderer/Swapchain.hpp"
#include "renderer/Renderer.hpp"
#include "ecs/Systems.hpp"
#include "core/Log.hpp"
#include <GLFW/glfw3.h>
#include <cassert>
#include <chrono>

namespace Vulkana {

Engine::Engine(Application& app)
    : m_app(app)
{
}

Engine::~Engine()
{
    cleanup();
}

// ------------------------------------------------------------------
// init - buat window, context, swapchain, renderer
// ------------------------------------------------------------------
void Engine::init()
{
    m_window = std::make_unique<Window>(1280, 720, "Vulkana");

    // Register input callbacks ke GLFW window
    GLFWwindow* handle = m_window->getHandle();
    glfwSetKeyCallback(handle, [](GLFWwindow*, int k, int s, int a, int m) {
        Input::keyCallback(k, s, a, m);
    });
    glfwSetMouseButtonCallback(handle, [](GLFWwindow*, int b, int a, int m) {
        Input::mouseButtonCallback(b, a, m);
    });
    glfwSetCursorPosCallback(handle, [](GLFWwindow*, double x, double y) {
        Input::cursorPosCallback(x, y);
    });

    m_context = std::make_unique<Context>();
    m_context->init(handle);

    // Buat surface dari GLFW window
    VkResult res = glfwCreateWindowSurface(
        m_context->instance(), handle, nullptr, &m_surface);
    (void)res;
    assert(res == VK_SUCCESS && "Gagal buat surface");

    int w, h;
    m_window->getFramebufferSize(w, h);

    m_swapchain = std::make_unique<Swapchain>();
    m_swapchain->init(m_context->physicalDevice(), m_context->device(),
                      m_surface, w, h);

    m_renderer = std::make_unique<Renderer>();
    m_renderer->init(*m_context, *m_swapchain);

    LOG_INFO("Engine siap");
    m_app.onInit();
}

void Engine::recreateSwapchain()
{
    int w = 0, h = 0;
    m_window->getFramebufferSize(w, h);
    if (w == 0 || h == 0) return;

    vkDeviceWaitIdle(m_context->device());
    m_swapchain->recreate(w, h);
    m_renderer->init(*m_context, *m_swapchain);
    LOG_INFO("Swapchain direcreate");
}

void Engine::cleanup()
{
    if (m_context && m_context->device())
        vkDeviceWaitIdle(m_context->device());

    m_app.onCleanup();
    m_renderer.reset();
    m_swapchain.reset();

    if (m_surface && m_context)
        vkDestroySurfaceKHR(m_context->instance(), m_surface, nullptr);

    m_context.reset();
    m_window.reset();
}

// ------------------------------------------------------------------
// run - loop utama dengan delta time
// ------------------------------------------------------------------
void Engine::run()
{
    init();

    auto lastTime = std::chrono::high_resolution_clock::now();

    while (!m_window->shouldClose())
    {
        m_window->pollEvents();

        // Hitung delta time
        auto now = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;

        // Update
        m_app.onUpdate(dt);
        Systems::updateTransforms(dt);

        // Cek resize window (from GLFW callback)
        if (Window::resized)
        {
            Window::resized = false;
            recreateSwapchain();
            continue;
        }

        // Render
        bool ok = m_renderer->beginFrame();
        if (!ok)
        {
            // Swapchain out of date, recreate
            recreateSwapchain();
            continue;
        }

        m_app.onRender();
        Systems::renderMeshes();

        if (!m_renderer->endFrame())
        {
            Window::resized = false;
            recreateSwapchain();
        }
        Input::endFrame();
    }

    cleanup();
}

}
