#include "core/Application.hpp"
#include "core/Engine.hpp"
#include "core/Log.hpp"

Application::Application()
    : m_engine(nullptr)
{
}

Application::~Application() {
    delete m_engine;
}

void Application::run() {
    Log::info("Aplikasi dimulai");

    m_engine = new Engine();
    if (!m_engine->init()) {
        Log::error("Inisialisasi engine gagal, keluar");
        return;
    }

    m_engine->run();
    m_engine->shutdown();

    Log::info("Aplikasi selesai");
}
