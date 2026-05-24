// Entry point - turunkan Application dan jalankan Engine

#include "core/Engine.hpp"
#include "core/Application.hpp"
#include "core/Log.hpp"
#include "core/Input.hpp"
#include "ecs/Systems.hpp"
#include <GLFW/glfw3.h>

// ==================================================================
// Contoh aplikasi sederhana
// ==================================================================
class MyApp : public Vulkana::Application
{
public:
    void onInit() override
    {
        LOG_INFO("Aplikasi Vulkana dimulai");
    }

    void onUpdate(float dt) override
    {
        (void)dt;

        if (Vulkana::Input::isKeyPressed(GLFW_KEY_ESCAPE))
        {
            LOG_INFO("ESC ditekan, keluar");
        }
    }

    void onRender() override
    {
        // Render dihandle oleh Engine -> Renderer
    }

    void onCleanup() override
    {
        LOG_INFO("Aplikasi Vulkana ditutup");
    }
};

// ==================================================================
// main
// ==================================================================
int main()
{
    MyApp app;
    Vulkana::Engine engine(app);
    engine.run();
    return 0;
}
