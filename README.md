# Vulkana — Game Engine Berbasis Vulkan

> **Proyek sampingan pribadi** — dibuat sebagai sarana belajar graphics programming
> dan pengembangan game engine menggunakan Vulkan API.

**Pembuat:** Mahasiswa Ilmu Politik, Fakultas Hukum, Universitas Jambi

**Jenis Engine:** Low-level rendering framework / game engine minimal.
Bukan engine siap-pakai seperti Unity atau Unreal — Vulkana adalah
kerangka kerja (framework) yang menangani boilerplate Vulkan
(instance, device, swapchain, pipeline, buffer) dan loop utama.
Anda tulis sendiri semua logic game, ECS, input handling, dan
rendering lewat callback `onInit`, `onUpdate`, `onRender`.

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

## Workflow Pengembangan Game

Berikut alur kerja yang disarankan untuk mengembangkan game dengan Vulkana:

### 1. Buat Kelas Game

Turunkan `Vulkana::Application` di `main.cpp`:

```cpp
#include "core/Engine.hpp"
#include "core/Application.hpp"
#include "core/Log.hpp"
#include "core/Input.hpp"
#include <GLFW/glfw3.h>

class Game : public Vulkana::Application
{
    // Simpan state game di sini
    float rotasi = 0.0f;

    void onInit() override
    {
        LOG_INFO("Game siap");
        // 1. Setup pipeline (buat shader, pipeline object)
        // 2. Setup vertex/index buffers
        // 3. Setup camera
    }

    void onUpdate(float dt) override
    {
        // Logic game per-frame
        rotasi += dt * 0.5f;

        if (Vulkana::Input::isKeyPressed(GLFW_KEY_ESCAPE))
            glfwSetWindowShouldClose(
                glfwGetCurrentContext(), GLFW_TRUE);
    }

    void onRender() override
    {
        // Render per-frame:
        // 1. Update push constants (MVP matrix)
        // 2. Bind pipeline
        // 3. Bind vertex/index buffers
        // 4. Draw
    }

    void onCleanup() override
    {
        LOG_INFO("Game ditutup");
        // Destroy semua Vulkan resources
    }
};

int main()
{
    Game game;
    Vulkana::Engine engine(game);
    engine.run();
    return 0;
}
```

### 2. Atur Urutan Init

Dalam `onInit()`, urutkan inisialisasi sesuai dependensi:

```
1. Pipeline  → 2. Buffers  → 3. Descriptor Sets
```

Akses device/context/swapchain lewat Engine jika perlu
(ditambahkan sendiri sesuai kebutuhan).

### 3. Buat Entity & Component

Gunakan EnTT untuk ECS:

```cpp
#include <entt/entt.hpp>
#include "ecs/Components.hpp"

entt::registry registry;

auto entity = registry.create();
registry.emplace<Vulkana::Transform>(entity,
    glm::vec3{0.0f, 0.0f, -5.0f},  // posisi
    glm::vec3{0.0f},                 // rotasi
    glm::vec3{1.0f}                  // scale
);
```

### 4. Buat Mesh

Isi data vertex langsung di kode atau load dari file:

```cpp
struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
};

std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    {{ 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    {{ 0.0f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
};

std::vector<uint32_t> indices = {0, 1, 2};
```

### 5. Setup Pipeline

Panggil `Vulkana::Pipeline::init()` di `onInit()`:

```cpp
Vulkana::Pipeline pipeline;
pipeline.init(device, renderPass,
    SHADER_DIR "/scene.vert.spv",
    SHADER_DIR "/scene.frag.spv",
    extent);
```

`SHADER_DIR` sudah didefinisikan oleh CMake — path absolut ke
folder `src/shaders/`.

### 6. Buat Vertex & Index Buffer

```cpp
Vulkana::Buffer vertexBuf, indexBuf;

vertexBuf.init(allocator, sizeof(Vertex) * vertices.size(),
    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
    vertices.data());

indexBuf.init(allocator, sizeof(uint32_t) * indices.size(),
    VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
    indices.data());
```

### 7. Render di `onRender()`

Record draw call ke command buffer yang sedang aktif:

```cpp
void onRender() override
{
    VkCommandBuffer cmd = ???; // Akses dari renderer

    // Push constant MVP
    glm::mat4 mvp = proj * view * model;
    vkCmdPushConstants(cmd, pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mvp), &mvp);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    VkBuffer vb[] = {vertexBuf.handle()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmd, 0, 1, vb, offsets);
    vkCmdBindIndexBuffer(cmd, indexBuf.handle(), 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(cmd, indexCount, 1, 0, 0, 0);
}
```

### 8. Input

Cek state input setiap frame di `onUpdate()`:

```cpp
if (Vulkana::Input::isKeyPressed(GLFW_KEY_W))
    player.moveForward(dt);

if (Vulkana::Input::isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
    player.attack();

auto mouse = Vulkana::Input::mouseDelta();
camera.rotate(mouse.y * 0.01f, mouse.x * 0.01f);
```

### 9. Build & Test

```bash
cmake --build build
./build/Debug/Vulkana.exe
```

### Ringkasan Alur

```
onInit()
  ├── Pipeline.init()
  ├── Buffer.init() x N
  └── Descriptor setup

onUpdate(dt)          ← tiap frame
  ├── Input cek
  ├── ECS update
  └── Logic game

onRender()            ← tiap frame
  ├── Push constants
  ├── Bind pipeline
  ├── Bind buffers
  └── Draw

onCleanup()
  └── Destroy semua resources
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
