#pragma once

struct GLFWwindow;

/**
 * Window - Abstraksi jendela OS menggunakan GLFW.
 */
class Window {
public:
    Window();
    ~Window();

    bool create(int width, int height, const char* title);
    void destroy();
    bool shouldClose() const;
    GLFWwindow* getHandle() const;
    int getWidth() const;
    int getHeight() const;
    bool wasResized() const;
    void resetResizedFlag();

private:
    GLFWwindow* m_window;
    int m_width;
    int m_height;
    bool m_framebufferResized;

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
};
