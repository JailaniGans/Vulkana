#pragma once

struct GLFWwindow;

/**
 * Input - Penangan input keyboard dan mouse via GLFW.
 */
class Input {
public:
    Input();
    ~Input();

    void init(GLFWwindow* window);
    void poll();
    bool isKeyPressed(int key) const;
    void getMousePos(double& x, double& y) const;
    bool isMouseButtonPressed(int button) const;

private:
    GLFWwindow* m_window;
};
