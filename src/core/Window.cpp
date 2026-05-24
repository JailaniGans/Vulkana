#include "core/Window.hpp"
#include "core/Log.hpp"
#include <GLFW/glfw3.h>
#include <cassert>

namespace Vulkana {

bool Window::resized = false;

Window::Window(int width, int height, std::string_view title)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
    assert(m_window && "Gagal membuat window GLFW");

    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
    LOG_INFO("Window dibuat");
}

Window::~Window()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
    LOG_INFO("Window dihapus");
}

bool Window::shouldClose() const
{
    return glfwWindowShouldClose(m_window);
}

void Window::pollEvents() const
{
    glfwPollEvents();
}

void Window::getFramebufferSize(int& width, int& height) const
{
    glfwGetFramebufferSize(m_window, &width, &height);
}

std::vector<const char*> Window::getRequiredExtensions() const
{
    uint32_t count = 0;
    const char** extensions = glfwGetRequiredInstanceExtensions(&count);
    return {extensions, extensions + count};
}

void Window::framebufferResizeCallback(GLFWwindow* window, int, int)
{
    auto* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win) resized = true;
}

}
