# 3D HUD Rendering Engine

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://en.cppreference.com/w/cpp/compiler_support)
[![CMake](https://img.shields.io/badge/CMake-3.16+-green.svg)](https://cmake.org/)
[![Conan](https://img.shields.io/badge/Conan-2.x-orange.svg)](https://conan.io/)
[![Platforms](https://img.shields.io/badge/platforms-Windows%20%7C%20Linux%20%7C%20Android%20%7C%20QNX-blue)]()

A high-performance, cross-platform 3D Heads-Up Display (HUD) rendering engine designed for automotive, aviation, and embedded systems applications. Built with modern C++17, featuring a clean layered architecture, multi-display support, and hardware-agnostic rendering interface.

## ✨ Features

### 🚀 **Performance-Oriented Design**
- **Zero-copy Command Buffer Architecture**: Eliminates unnecessary memory copies during command recording and execution
- **Multi-threaded Rendering**: Support for concurrent command recording and execution across multiple threads
- **Efficient Resource Management**: Smart resource pooling with shared/private context isolation
- **Priority-based Command Execution**: High-priority commands (state changes, clears) execute before normal operations

### 🌐 **Cross-Platform Support**
- **Windows**: Win32/WGL with OpenGL 4.5
- **Linux**: X11/GLX with OpenGL 4.5
- **Android**: EGL with OpenGL ES 3.2
- **QNX**: EGL with OpenGL ES 3.2
- **Unified API**: Single interface across all platforms

### 🏗️ **Modern Architecture**
- **Layered Design**: Clean separation between platform, RHI, rendering, and application layers
- **API-Agnostic RHI**: Abstract Render Hardware Interface supporting multiple graphics APIs
- **Component-Based**: Independent systems for scene management, material handling, and rendering
- **Type Safety**: Strongly typed resource handles and enumerations

### 🎮 **Advanced Rendering Features**
- **Multi-Window Rendering**: Simultaneous rendering to multiple physical displays (up to 8 windows)
- **Multi-Viewport Support**: Multiple views per window (3D view, minimap, UI overlays)
- **Modern Material System**: Real-time material editing with hot reload capability
- **Post-Processing Pipeline**: Built-in effects (Bloom, Tone Mapping, etc.)
- **Debug Visualization**: Wireframe, bounding boxes, normals, and performance overlays

## 🎯 Target Applications

### **Automotive HUD Systems**
- **Windshield Projection**: 3D navigation, speed, and safety information
- **Instrument Clusters**: Digital gauge clusters with 3D effects
- **Infotainment Displays**: Rich media interfaces with hardware acceleration
- **Advanced Driver Assistance Systems (ADAS)**: Visual alerts and guidance

### **Aviation & Aerospace**
- **Flight Deck Displays**: Primary flight displays (PFD), navigation displays
- **Helmet-Mounted Displays**: Augmented reality for pilots
- **Mission Control Systems**: Multi-monitor visualization systems

### **Embedded & Industrial**
- **Medical Devices**: Surgical navigation, diagnostic imaging
- **Industrial Control**: SCADA systems, process visualization
- **Digital Signage**: High-resolution advertising displays

### **General Purpose**
- **Simulation & Training**: Flight simulators, driving simulators
- **Game Development**: Engine for 3D games and applications
- **Scientific Visualization**: Data visualization, virtual reality

## 🏗️ Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                  Application Layer                          │
│  (Scene, Camera, Animation, Game Logic)                    │
└────────────────────────┬────────────────────────────────────┘
                         │
┌────────────────────▼────────────────────────────────────────┐
│                Rendering Engine                             │
│  ┌──────────────┬──────────────┬─────────────────┐         │
│  │ WindowManager │ ViewManager  │ Renderer       │         │
│  └──────────────┴──────────────┴─────────────────┘         │
└────────────────────┬────────────────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────────────────┐
│              Command Buffer System                          │
│  ┌─────────────────┬─────────────────┬─────────────┐       │
│  │ CommandBuffer    │ BufferManager   │ Priority     │       │
│  │ (per-window)    │ (pool)         │ Executor     │       │
│  └─────────────────┴─────────────────┴─────────────┘       │
└────────────────────┬────────────────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────────────────┐
│                   RHI Layer                                 │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ IRHIDevice (Device Abstraction)                      │  │
│  │  ┌──────────────────────────────────────────────┐  │  │
│  │  │ IResourceManager (Multi-context Resource Mgmt)│  │  │
│  │  │  - Shared Resources (Textures, Shaders, Buffers)│  │  │
│  │  │  - Private Resources (FBOs, VAOs - per-context) │  │  │
│  │  │  - Context Association Management              │  │  │
│  │  └──────────────────────────────────────────────┘  │  │
│  │  - API-agnostic Render Commands                    │  │
│  │  - Multi-window/Context Support                    │  │
│  │  - Depends on IGraphicsContext                     │  │
│  └──────────────────────────────────────────────────────┘  │
└────────────────────┬────────────────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────────────────┐
│         Platform Graphics Context Layer                     │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ IGraphicsContext (Platform Abstraction)              │  │
│  │  - Create/Destroy platform-specific render contexts  │  │
│  │  - Window binding (HWND, X11 Window, EGLSurface)     │  │
│  │  - Buffer swapping (SwapBuffers)                     │  │
│  │  - VSync control                                     │  │
│  │  - Context switching (MakeCurrent)                   │  │
│  └──────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

### **Key Components**

1. **Platform Abstraction Layer**
   - `IGraphicsContext`: Platform-specific graphics context management
   - `IWindowManager`: Window creation and event handling
   - Abstract implementations for Win32/WGL, X11/GLX, and EGL

2. **Render Hardware Interface (RHI) Layer**
   - `IRHIDevice`: Device abstraction with API-agnostic render commands
   - `IResourceManager`: Multi-context resource management with shared/private pools
   - Strongly typed resource handles for textures, shaders, buffers, etc.

3. **Command Buffer System**
   - `ICommandBuffer`: Per-window command recording and execution
   - Priority-based execution ordering
   - Memory-efficient buffer pooling

4. **Rendering Engine Layer**
   - `IRenderManager`: Central rendering control and frame management
   - `ISceneManager`: Scene graph and object hierarchy management
   - `IMaterialSystem`: Material, shader, and technique management

## 📦 Installation

### Prerequisites

- **CMake 3.16+**
- **C++17 compatible compiler**
- **Conan 2.x** (dependency management)
- **Graphics Drivers**: OpenGL 4.5 / OpenGL ES 3.2 support

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/yourusername/3d_hud.git
cd 3d_hud

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the project
cmake --build . --config Release

# Alternatively, use the provided build scripts
./build.sh        # Linux/macOS
build.bat         # Windows
python build.py   # Cross-platform Python script
```

### Dependencies

The project uses Conan for dependency management. Key dependencies include:

- **OpenGL/OpenGL ES**: Graphics API (system provided)
- **GLAD**: OpenGL loading library
- **GLM**: Mathematics library
- **spdlog**: Logging library
- **stb_image**: Image loading

## 🚀 Quick Start

```cpp
#include "rendering/render_manager.h"
#include "rendering/scene_manager.h"

using namespace hud_3d::rendering;

int main() {
    // Create render manager
    auto renderer = CreateRenderManager();
    
    // Initialize with OpenGL backend
    if (!renderer->Initialize(BackendType::OpenGL)) {
        return 1;
    }
    
    // Setup scene
    auto sceneManager = renderer->GetSceneManager();
    auto materialSystem = renderer->GetMaterialSystem();
    
    // Create a simple cube
    MeshData cube = CreateCubeMesh();
    ResourceHandle cubeHandle = renderer->CreateMesh(cube);
    
    // Main render loop
    while (running) {
        // Begin new frame
        renderer->BeginFrame();
        
        // Clear screen
        renderer->Clear(Color::Blue());
        
        // Begin scene rendering
        renderer->BeginScene();
        
        // Render objects
        Transform cubeTransform;
        cubeTransform.position = {0.0f, 0.0f, -5.0f};
        renderer->DrawMesh(cubeHandle, cubeTransform);
        
        // End scene and frame
        renderer->EndScene();
        renderer->EndFrame();
    }
    
    // Cleanup
    renderer->Shutdown();
    return 0;
}
```

## 📁 Project Structure

```
3d_hud/
├── CMakeLists.txt          # Root CMake configuration
├── build.py                # Cross-platform build script
├── build.sh                # Linux/macOS build script
├── build.bat               # Windows build script
├── conanfile.txt           # Conan dependency configuration
├── docs/                   # Design documents
│   └── DESIGN.md           # Comprehensive design documentation
├── inc/                    # Header files
│   ├── platform/           # Platform abstraction layer
│   │   ├── graphics_context.h
│   │   ├── window_manager.h
│   │   └── window.h
│   ├── rendering/          # Rendering engine
│   │   ├── rhi/            # Render Hardware Interface
│   │   │   ├── rhi_device.h
│   │   │   ├── rhi_types.h
│   │   │   └── rhi_factory.h
│   │   ├── render_manager.h
│   │   ├── scene_manager.h
│   │   ├── material_system.h
│   │   └── rendering_types.h
│   └── utils/              # Utility libraries
│       ├── log/            # Logging system
│       ├── memory/         # Memory management
│       ├── math/           # Mathematics library
│       └── perf/           # Performance profiling
├── src/                    # Source files
│   ├── platform/           # Platform implementations
│   ├── rendering/          # Rendering implementations
│   └── utils/              # Utility implementations
├── examples/               # Usage examples
│   └── rendering_usage.cpp # Comprehensive usage example
└── cmake/                  # CMake modules
    ├── build.cmake         # Compiler-specific settings
    ├── dir.cmake           # Directory structure
    ├── lib.cmake           # Library configuration
    └── option.cmake        # Build options
```

## 🎯 Key Design Patterns

### **Strategy Pattern**
- Abstract RHI implementations for different graphics APIs
- Platform-specific context creation strategies

### **Factory Pattern**
- Creation of render managers, scenes, and materials
- Backend-specific factory methods

### **Component Pattern**
- Independent rendering systems (scene, material, viewport)
- Loose coupling between engine components

### **Observer Pattern**
- Event handling for window input and system events
- Scene change notifications

## 🔧 Configuration Options

### **CMake Options**
```bash
# Enable/disable features
cmake .. -DUSE_VULKAN=OFF \
         -DUSE_D3D12=OFF \
         -DENABLE_DEBUG=ON \
         -DENABLE_PROFILING=OFF

# Platform-specific settings
cmake .. -DPLATFORM=WINDOWS \
         -DBUILD_SHARED_LIBS=OFF \
         -DCMAKE_INSTALL_PREFIX=./install
```

### **Runtime Configuration**
- Backend selection (OpenGL, Vulkan, Direct3D)
- Window count and display arrangement
- Rendering quality settings
- Debug visualization toggles

## 📊 Performance Considerations

### **Memory Optimization**
- **Resource Pooling**: Reuse of GPU resources to minimize allocations
- **Command Buffer Recycling**: Pre-allocated command buffers with dynamic resizing
- **Batch Rendering**: Combine draw calls for similar objects

### **CPU Optimization**
- **Multi-threaded Command Recording**: Concurrent command buffer population
- **Async Resource Loading**: Non-blocking texture and mesh loading
- **Scene Graph Culling**: Hierarchical visibility determination

### **GPU Optimization**
- **State Sorting**: Minimize pipeline state changes
- **Instanced Rendering**: Single draw call for multiple identical objects
- **Texture Atlasing**: Combine small textures into larger ones

## 🐛 Debugging & Profiling

### **Built-in Debug Features**
```cpp
// Enable debug output
device->EnableDebugOutput(true);

// Add debug markers
device->PushDebugGroup("MainRenderPass");
// ... rendering commands ...
device->PopDebugGroup();

// Performance statistics
RenderStats stats = renderer->GetStats();
std::cout << "Frame time: " << stats.frameTime << "ms\n"
          << "Draw calls: " << stats.drawCalls << "\n"
          << "Triangles: " << stats.trianglesDrawn << std::endl;
```

### **Profiling Tools**
- **CPU Profiler**: Hierarchical timing of system components
- **GPU Profiler**: OpenGL timer queries for GPU performance
- **Memory Profiler**: Tracking of resource allocations and usage

## 📚 API Documentation

### **Core Interfaces**

#### **IRenderManager**
```cpp
// Frame management
bool BeginFrame();
bool EndFrame();

// Scene management
void BeginScene();
void EndScene();

// Resource creation
ResourceHandle CreateMesh(const MeshData& data);
ResourceHandle CreateTexture(const TextureData& data);
ResourceHandle CreateShader(const ShaderData& data);

// Drawing operations
void DrawMesh(ResourceHandle mesh, const Transform& transform);
```

#### **ISceneManager**
```cpp
// Scene graph operations
ISceneNode* CreateNode(const std::string& name);
bool RemoveNode(ISceneNode* node);

// Object management
ResourceHandle AddObject(const std::string& meshPath, const Transform& transform);
void UpdateObjectTransform(ResourceHandle object, const Transform& transform);
```

#### **IMaterialSystem**
```cpp
// Material operations
ResourceHandle CreateMaterial(const std::string& name);
void SetMaterialDiffuse(ResourceHandle material, const Color& color);
void SetMaterialTexture(ResourceHandle material, TextureSlot slot, ResourceHandle texture);

// Shader management
ResourceHandle LoadShader(const std::string& vertexPath, const std::string& fragmentPath);
bool ReloadShader(ResourceHandle shader);
```

## 🔮 Roadmap

### **Short-term Goals**
- [ ] Vulkan backend implementation
- [ ] Enhanced post-processing effects
- [ ] VR/AR support framework
- [ ] Particle system integration

### **Medium-term Goals**
- [ ] Direct3D 12 backend
- [ ] Ray tracing support
- [ ] Advanced shadow techniques
- [ ] Physics-based rendering

### **Long-term Vision**
- [ ] Cloud rendering support
- [ ] AI-powered content generation
- [ ] Cross-platform shader compilation
- [ ] Procedural content generation system

## 🤝 Contributing

We welcome contributions! Please see our [Contributing Guidelines](CONTRIBUTING.md) for details.

1. **Fork** the repository
2. **Create** a feature branch (`git checkout -b feature/amazing-feature`)
3. **Commit** your changes (`git commit -m 'Add amazing feature'`)
4. **Push** to the branch (`git push origin feature/amazing-feature`)
5. **Open** a Pull Request

### **Development Setup**
```bash
# Install development dependencies
conan install . --build=missing

# Configure with development options
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_TESTS=ON

# Run tests
ctest --output-on-failure
```

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **OpenGL/OpenGL ES**: The foundation of our graphics implementation
- **GLM**: Mathematics library for 3D transformations
- **spdlog**: Fast logging library
- **stb_image**: Single-header image loading library
- **Conan**: Dependency management made easy

---

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/yourusername/3d_hud/issues)
- **Discussions**: [GitHub Discussions](https://github.com/yourusername/3d_hud/discussions)
- **Email**: support@3dhud.example.com

## 📊 Stats

![Project Status](https://img.shields.io/badge/status-active-success.svg)
![Last Commit](https://img.shields.io/github/last-commit/yourusername/3d_hud.svg)
![Open Issues](https://img.shields.io/github/issues/yourusername/3d_hud.svg)

---

**Built with passion for real-time graphics and clean software architecture.**