#pragma once

class Renderer {
public:
    Renderer();
    ~Renderer();
    bool init();
    void render();
    void cleanup();
};
