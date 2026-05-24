#pragma once

class Context {
public:
    Context();
    ~Context();
    bool init();
    void cleanup();
};
