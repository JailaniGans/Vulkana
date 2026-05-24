#pragma once

#include <entt/entt.hpp>

class Window;
class Input;
class RenderSystem;
class PhysicsSystem;

/**
 * Engine - Orkestrator inti game engine.
 * Memiliki Window, Input, ECS registry, dan sistem game.
 * Siklus hidup: init() -> run() -> shutdown()
 */
class Engine {
public:
    Engine();
    ~Engine();

    bool init();
    void run();
    void shutdown();

private:
    Window*          m_window;
    Input*           m_input;
    entt::registry   m_registry;
    RenderSystem*    m_renderSystem;
    PhysicsSystem*   m_physicsSystem;
    float            m_lastTime;
};
