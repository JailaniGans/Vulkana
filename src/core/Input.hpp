#pragma once

// Input - state keyboard & mouse dari callback GLFW
//   Panggil Input::update() setiap akhir frame

#include <array>
#include <cstdint>

namespace Vulkana {

struct MousePosition {
    double x = 0.0;
    double y = 0.0;
};

class Input {
public:
    static void keyCallback(int key, int scancode, int action, int mods);
    static void mouseButtonCallback(int button, int action, int mods);
    static void cursorPosCallback(double xpos, double ypos);

    static bool isKeyPressed(int key);
    static bool isMouseButtonPressed(int button);
    static MousePosition mousePosition();
    static MousePosition mouseDelta();

    static void endFrame();

private:
    static std::array<bool, 512> s_keys;
    static std::array<bool, 8> s_mouseButtons;
    static MousePosition s_pos;
    static MousePosition s_delta;
};

}
