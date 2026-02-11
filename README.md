# DirectX 12 Real-Time Path Tracer

A from-scratch implementation of a real-time path tracer using DirectX 12 compute shaders. Built to explore modern GPU programming, performance optimization, and the challenges of achieving interactive framerates with physically-based rendering.

**Project Goal:** Real-time performance (30-60 FPS) with progressive quality improvement, not offline high-quality renders.

## Demo

_Screenshots and demo videos coming soon_

## Features

- **DirectX 12 Foundation** - Device initialization, command queues, swap chain, and resource management
- **Debug Infrastructure** - DX12 debug layer integration with info queue for comprehensive error reporting
- **Window Management** - Native Windows window with resize support
- **Compute Shader Path Tracing** (WIP) - GPU-accelerated ray tracing using HLSL compute shaders
- **Real-Time Rendering** (WIP) - Progressive accumulation and temporal reprojection for interactive framerates
- **BVH Acceleration Structure** (WIP) - Efficient scene traversal for complex geometry
- **PBR Materials** (WIP) - Physically-based materials with importance sampling
- **Denoising** (WIP) - Spatial and temporal denoising for low sample counts
- **PIX Integration** (WIP) - GPU profiling and performance analysis

## Technology Stack

- **DirectX 12** - Graphics API for GPU acceleration
- **HLSL** - High-Level Shader Language for compute shaders
- **C++23** - Modern C++ with latest language features
- **CMake** - Cross-platform build system
- **GLM** - Mathematics library for vectors and matrices
- **stb_image** - Image I/O library

### Development Tools

- **PIX for Windows** - GPU profiling and debugging (WIP)
- **Windows Performance Toolkit** - System-level performance analysis (WIP)
- **Visual Studio 2022** - Primary development environment

## Prerequisites

### Software Requirements

- **Operating System:** Windows 10/11 (version 2004 or later)
- **Compiler:** Visual Studio 2022 (17.8 or later) with C++ Desktop Development workload
- **Windows SDK:** 10.0.22000.0 or later
- **Build Tools:** CMake 3.24 or later

### Hardware Requirements

- **GPU:** DirectX 12 capable graphics card
  - **Recommended:** NVIDIA RTX 3060 / AMD RX 6600 or better
  - **Minimum:** Any DX12 compatible GPU (performance will vary)
- **RAM:** 8GB minimum, 16GB recommended
- **Storage:** ~500MB for build artifacts

## Building

```bash
# Clone the repository
git clone <repository-url>
cd pathtracer

# Configure with CMake
cmake -B build -S .

# Build (Debug)
cmake --build build --config Debug

# Or build (Release)
cmake --build build --config Release
```

### Build Configurations

| Configuration | Description |
| --- | --- |
| **Debug** | Full debug information, all validation layers enabled |
| **Release** | Optimized for performance testing |
| **RelWithDebInfo** | Release optimizations with debug symbols |
| **MinSizeRel** | Optimized for size |

## Running

After building, run the executable:

```bash
# Debug build
./build/bin/Debug/pathtracer.exe

# Release build
./build/bin/Release/pathtracer.exe
```

The application will open a window and display the rendered scene. Currently displays a cornflower blue clear color as a foundation test.

## Contributing

This is a personal learning project and I'm not accepting pull requests at this time. However, feedback, suggestions, and discussions are always welcome! Feel free to open an issue if you spot a bug, have an optimization idea, or want to discuss rendering techniques.

## License

MIT License - See LICENSE file for details.
