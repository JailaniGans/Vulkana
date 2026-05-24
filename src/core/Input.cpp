#include "core/Input.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

Input::Input()
    : m_window(nullptr)
{
}

Input::~Input() {
}

void Input::init(GLFWwindow* window) {
    m_window = window;
}

void Input::poll() {
    glfwPollEvents();
}

bool Input::isKeyPressed(int key) const {
    if (!m_window) return false;
    return glfwGetKey(m_window, key) == GLFW_PRESS;
}

void Input::getMousePos(double& x, double& y) const {
    if (m_window) {
        glfwGetCursorPos(m_window, &x, &y);
    } else {
        x = 0.0;
        y = 0.0;
    }
}

bool Input::isMouseButtonPressed(int button) const {
    if (!m_window) return false;
    return glfwGetMouseButton(m_window, button) == GLFW_PRESS;
}
