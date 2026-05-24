/**
 * Vulkana - Titik masuk engine game.
 */
#include "core/Application.hpp"
#include "core/Log.hpp"

int main(int argc, char** argv) {
    Log::init();
    Log::info("Vulkana Engine v0.1.0 dimulai...");

    Application app;
    app.run();

    Log::info("Vulkana Engine selesai");
    return 0;
}
