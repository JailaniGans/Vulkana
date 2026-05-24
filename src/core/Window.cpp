#include "core/Window.hpp"
#include "core/Log.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

Window::Window()
    : m_window(nullptr)
    , m_width(0)
    , m_height(0)
    , m_framebufferResized(false)
{
}

Window::~Window() {
    destroy();
}

bool Window::create(int width, int height, const char* title) {
    m_width = width;
    m_height = height;

    if (!glfwInit()) {
        Log::error("Gagal menginisialisasi GLFW");
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(m_width, m_height, title, nullptr, nullptr);
    if (!m_window) {
        Log::error("Gagal membuat jendela GLFW");
        glfwTerminate();
        return false;
    }

    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);

    Log::info("Jendela dibuat");
    return true;
}

void Window::destroy() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_window);
}

GLFWwindow* Window::getHandle() const {
    return m_window;
}

int Window::getWidth() const {
    return m_width;
}

int Window::getHeight() const {
    return m_height;
}

bool Window::wasResized() const {
    return m_framebufferResized;
}

void Window::resetResizedFlag() {
    m_framebufferResized = false;
}

void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win) {
        win->m_width = width;
        win->m_height = height;
        win->m_framebufferResized = true;
    }
}
