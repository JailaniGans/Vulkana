# Vulkana — Game Engine Berbasis Vulkan

> **Proyek sampingan pribadi** — dibuat sebagai sarana belajar graphics programming
> dan pengembangan game engine menggunakan Vulkan API.

**Pembuat:** Mahasiswa Ilmu Politik, Fakultas Hukum, Universitas Jambi

---

## Persyaratan Sistem

Tidak perlu install Vulkan SDK, GLFW, atau library apapun.
Cukup:

| Kebutuhan | Minimal |
|-----------|---------|
| Compiler | C++17 (MSVC, GCC, Clang) |
| Build System | CMake >= 3.10 |
| GPU | Vulkan 1.3 (semua GPU modern) |
| OS | Windows 10+, Linux, macOS |

Semua dependency sudah include sebagai Git submodule di `ext/`:

| Library | Versi | Fungsi |
|---------|-------|--------|
| GLFW | latest | Window & input |
| GLM | latest | Matriks & vektor |
| Volk | latest | Loader Vulkan runtime |
| VMA | 3.4.x | Manajemen memory GPU |
| EnTT | latest | ECS (Entity-Component-System) |
| Vulkan-Headers | latest | Header Vulkan API |

---

## Cara Clone & Build

```bash
# 1. Clone repository
git clone https://github.com/anakb/Vulkana.git
cd Vulkana

# 2. Init submodule (download semua dependency)
git submodule update --init --recursive

# 3. Configure & build
cmake -S . -B build
cmake --build build

# 4. Jalankan
./build/Debug/Vulkana.exe
```

Di Linux/macOS:
```bash
cmake -S . -B build
cmake --build build
./build/Vulkana
```

---

## Struktur Folder

```
Vulkana/
├── CMakeLists.txt          # Build system
├── README.md
├── assets/                 # Aset game (kosong, isi sendiri)
│   ├── models/
│   ├── textures/
│   └── audio/
├── build/                  # Output kompilasi (git-ignored)
├── ext/                    # Dependency (Git submodules)
│   ├── entt/               #   ECS
│   ├── glfw/               #   GLFW (build from source)
│   ├── glm/                #   GLM (header-only)
│   ├── VMA/                #   Vulkan Memory Allocator
│   ├── Volk/               #   Volk (loader runtime)
│   └── Vulkan-Headers/     #   Header Vulkan API
└── src/
    ├── main.cpp            # Entry point
    ├── core/               # Inti engine
    │   ├── Application.hpp/.cpp   # Base class aplikasi
    │   ├── Engine.hpp/.cpp        # Loop utama
    │   ├── Window.hpp/.cpp        # GLFW wrapper
    │   ├── Input.hpp/.cpp         # Keyboard & mouse
    │   └── Log.hpp/.cpp           # Logger
    ├── ecs/                # Entity-Component-System
    │   ├── Components.hpp  # Transform, Mesh, Camera
    │   └── Systems.hpp/.cpp
    ├── renderer/           # Vulkan renderer
    │   ├── Context.hpp/.cpp       # Instance, device, VMA
    │   ├── Swapchain.hpp/.cpp     # Swapchain, render pass
    │   ├── Pipeline.hpp/.cpp      # Shader & graphics pipeline
    │   ├── Buffer.hpp/.cpp        # Vertex/index/uniform buffer
    │   ├── Descriptor.hpp/.cpp    # Descriptor set
    │   ├── Renderer.hpp/.cpp      # Command buffer, present
    │   └── VMA.cpp                # Implementasi VMA
    └── shaders/            # Shader GLSL + SPIR-V
        ├── scene.vert            # Vertex shader
        ├── scene.frag            # Fragment shader
        ├── scene.vert.spv        # Pre-compiled SPIR-V
        ├── scene.frag.spv        # Pre-compiled SPIR-V
        └── compile.bat           # Script kompilasi (optional)
```

---

## Cara Pakai

Buat kelas turunan `Vulkana::Application`:

```cpp
#include "core/Engine.hpp"
#include "core/Application.hpp"
#include "core/Log.hpp"

class MyGame : public Vulkana::Application
{
    void onInit() override
    {
        LOG_INFO("Game start");
    }

    void onUpdate(float dt) override
    {
        // Update logic game dengan delta time
    }

    void onRender() override
    {
        // Render per-frame
    }

    void onCleanup() override
    {
        LOG_INFO("Game closed");
    }
};

int main()
{
    MyGame app;
    Vulkana::Engine engine(app);
    engine.run();
    return 0;
}
```

Semua callback sudah dipanggil otomatis oleh `Engine::run()`.

---

## Compile Shader (Opsional)

Shader `.vert`/`.frag` sudah di-pre-compile ke `.spv`.
Jika ingin mengubah shader dan punya Vulkan SDK:

```bash
# Otomatis via CMake (jika glslc di PATH)
cmake --build build

# Atau manual
cd src/shaders
glslc scene.vert -o scene.vert.spv
glslc scene.frag -o scene.frag.spv
```

---

## Catatan

- Engine menggunakan **Volk** untuk load function pointer Vulkan saat runtime.
  Tidak perlu link ke `vulkan-1.lib`.
- **VMA** dipakai untuk semua alokasi memory GPU.
- **Push constant** untuk MVP matrix — tidak perlu uniform buffer.
- Window title: "Vulkana", ukuran default: 1280×720.

---

## Credit

Vulkana © 2026 — Proyek sampingan sebagai sarana belajar pemrograman grafis.

Dibuat dengan ❤️ oleh seorang mahasiswa Ilmu Politik, Fakultas Hukum, Universitas Jambi.

---

## Lisensi

MIT
