#pragma once

class Engine;

/**
 * Application - Tingkat atas aplikasi. Membuat dan memiliki Engine.
 */
class Application {
public:
    Application();
    ~Application();

    void run();

private:
    Engine* m_engine;
};
