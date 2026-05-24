// ===================================================================
// Vulkana Game Client Entry Point
// ===================================================================
// Instantiates the Game class and runs the engine.
// ===================================================================

#include "Game.hpp"
#include "core/Engine.hpp"
#include "core/Log.hpp"

int main()
{
    try
    {
        Game game;
        Vulkana::Engine engine(game);
        engine.run();
    }
    catch (const std::exception&)
    {
        LOG_ERROR("Fatal error occurred");
        return 1;
    }
    return 0;
}
