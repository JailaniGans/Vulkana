#include "core/Input.hpp"

namespace Vulkana {

std::array<bool, 512> Input::s_keys{};
std::array<bool, 8> Input::s_mouseButtons{};
MousePosition Input::s_pos{};
MousePosition Input::s_delta{};

void Input::keyCallback(int key, int, int action, int)
{
    if (key >= 0 && key < static_cast<int>(s_keys.size()))
        s_keys[key] = (action != 0);
}

void Input::mouseButtonCallback(int button, int action, int)
{
    if (button >= 0 && button < static_cast<int>(s_mouseButtons.size()))
        s_mouseButtons[button] = (action != 0);
}

void Input::cursorPosCallback(double xpos, double ypos)
{
    s_delta.x += xpos - s_pos.x;
    s_delta.y += ypos - s_pos.y;
    s_pos.x = xpos;
    s_pos.y = ypos;
}

bool Input::isKeyPressed(int key)
{
    if (key >= 0 && key < static_cast<int>(s_keys.size()))
        return s_keys[key];
    return false;
}

bool Input::isMouseButtonPressed(int button)
{
    if (button >= 0 && button < static_cast<int>(s_mouseButtons.size()))
        return s_mouseButtons[button];
    return false;
}

MousePosition Input::mousePosition()
{
    return s_pos;
}

MousePosition Input::mouseDelta()
{
    return s_delta;
}

void Input::endFrame()
{
    s_delta = {0.0, 0.0};
}

}
