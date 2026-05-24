# Vulkana

Vulkan-based game engine project with ECS architecture.

## Prerequisites

- **Vulkan SDK** 1.4+ — set `VULKAN_SDK` env var (terdeteksi otomatis)
- **CMake** 3.10+
- **C++17** compatible compiler (MSVC 2022 / GCC / Clang)
- **GLFW** 3.4 — prebuilt binary di `C:\glfw-3.4.bin.WIN64\glfw-3.4.bin.WIN64`

## Clone & Setup

```bash
git clone --recursive https://github.com/JailaniGans/Vulkana.git
cd Vulkana

# Jika sudah clone tanpa --recursive:
git submodule update --init --recursive
```

## Build

```bash
cmake -S . -B build
cmake --build build
```

Atau buka `build/Vulkana.sln` di Visual Studio.

## Struktur Project

```
Vulkana/
├── assets/              # Sumber daya mentah (model, texture, audio)
├── cmake/               # CMake modules
├── ext/                 # Third-party libraries (submodules)
│   ├── glfw/            # Windowing & input
│   ├── glm/             # Linear algebra
│   ├── Volk/            # Vulkan meta-loader
│   ├── VMA/             # Vulkan Memory Allocator
│   └── entt/            # Entity-Component System
└── src/                 # Source code
    ├── main.cpp         # Entry point
    ├── core/            # Engine core (Application, Engine, Window, Input, Log)
    ├── ecs/             # Entity-Component System (Components, Systems)
    ├── renderer/        # Vulkan HAL (Context, Swapchain, Pipeline, Buffer, Descriptor, Renderer)
    └── shaders/         # GLSL shaders (compile.bat -> SPIR-V)
```

## Arsitektur

**Layered Architecture** — game logic (ECS) terpisah dari rendering (Vulkan HAL):

```
Application
  └── Engine
       ├── Window
       ├── Input
       ├── Log
       ├── ECS (Components + Systems)
       └── Renderer
            ├── Context
            ├── Swapchain
            ├── Pipeline
            ├── Buffer
            └── Descriptor
```

Setiap komponen di `src/renderer/` memiliki **satu tanggung jawab** (Single Responsibility), memudahkan debugging dan pengembangan.

## Dependencies

| Library | Lokasi | Fungsi |
|---|---|---|
| GLFW | `ext/glfw` | Window creation, input |
| GLM | `ext/glm` | Matriks/vektor math |
| Volk | `ext/Volk` | Vulkan function loader |
| VMA | `ext/VMA` | Vulkan memory management |
| EnTT | `ext/entt` | Entity-Component System |
