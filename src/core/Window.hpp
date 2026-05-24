#pragma once

// Window - bungkus GLFW, callback resize, sediakan extension Vulkan

#include <string_view>
#include <vector>

struct GLFWwindow;

namespace Vulkana {

class Window {
public:
    Window(int width, int height, std::string_view title);
    ~Window();

    bool shouldClose() const;
    void pollEvents() const;
    void getFramebufferSize(int& width, int& height) const;

    GLFWwindow* getHandle() const { return m_window; }
    std::vector<const char*> getRequiredExtensions() const;

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    static bool resized;

private:
    GLFWwindow* m_window = nullptr;
};

}
