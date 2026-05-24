#pragma once

// Application - kelas dasar abstrak untuk aplikasi pengguna
//   Turunkan kelas ini dan override callback:
//     onInit(), onUpdate(dt), onRender(), onCleanup()

namespace Vulkana {

class Application {
public:
    virtual ~Application() = default;

    virtual void onInit() = 0;
    virtual void onUpdate(float deltaTime) = 0;
    virtual void onRender() = 0;
    virtual void onCleanup() = 0;
};

}
