#include "core/Engine.hpp"
#include "core/Window.hpp"
#include "core/Input.hpp"
#include "core/Log.hpp"
#include "ecs/Systems.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

Engine::Engine()
    : m_window(nullptr)
    , m_input(nullptr)
    , m_renderSystem(nullptr)
    , m_physicsSystem(nullptr)
    , m_lastTime(0.0f)
{
}

Engine::~Engine() {
    shutdown();
}

bool Engine::init() {
    Log::info("Engine menginisialisasi...");

    m_window = new Window();
    if (!m_window->create(1280, 720, "Vulkana")) {
        Log::error("Gagal membuat jendela");
        return false;
    }

    m_input = new Input();
    m_input->init(m_window->getHandle());

    m_renderSystem = new RenderSystem();
    if (!m_renderSystem->init(m_window)) {
        Log::error("Gagal menginisialisasi sistem render");
        return false;
    }

    m_physicsSystem = new PhysicsSystem();

    Log::info("Engine berhasil diinisialisasi");
    return true;
}

void Engine::run() {
    Log::info("Engine memulai game loop");

    m_lastTime = static_cast<float>(glfwGetTime());

    while (!m_window->shouldClose()) {
        float waktuSekarang = static_cast<float>(glfwGetTime());
        float deltaTime = waktuSekarang - m_lastTime;
        m_lastTime = waktuSekarang;

        m_input->poll();
        m_physicsSystem->update(m_registry, deltaTime);
        m_renderSystem->update(m_registry);
    }

    m_renderSystem->waitIdle();
}

void Engine::shutdown() {
    Log::info("Engine mematikan...");

    delete m_renderSystem;
    m_renderSystem = nullptr;

    delete m_physicsSystem;
    m_physicsSystem = nullptr;

    delete m_input;
    m_input = nullptr;

    if (m_window) {
        m_window->destroy();
        delete m_window;
        m_window = nullptr;
    }

    Log::info("Engine dimatikan");
}
