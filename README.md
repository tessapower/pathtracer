# DirectX 12 Real-Time Path Tracer

A from-scratch implementation of a real-time path tracer using DirectX 12 compute shaders. Built to explore modern GPU programming, performance optimization, and the challenges of achieving interactive framerates with physically-based rendering.

## Project Status

**Work in Progress** - Currently evolving from a basic CPU ray tracer to a fully-featured GPU-accelerated real-time renderer.

## Why This Project?

I enjoy building graphics engines from the ground up to deeply understand how they work. This project explores:

- How to architect a real-time path tracer using DirectX 12
- Compute shader programming for ray tracing without DXR
- Performance optimization techniques to achieve interactive framerates
- Professional debugging and profiling workflows
- The trade-offs between quality and performance in real-time rendering

The goal is **real-time performance** (30-60 FPS) with progressive quality improvement, not offline high-quality renders.

## Technical Focus Areas

### Real-Time Rendering

- Low sample counts per frame (1-4 SPP)
- Progressive accumulation for static scenes
- Temporal reprojection for moving cameras
- Target: 30+ FPS at 1080p for moderately complex scenes
- Denoising to compensate for low sample counts

### GPU Compute Architecture

- DirectX 12 compute shaders (exploring both compute and DXR paths)
- Efficient resource and memory management
- Bounding Volume Hierarchies for acceleration
- Asynchronous compute for parallelism

### Performance Engineering

- Comprehensive profiling with PIX and Windows Performance Toolkit
- GPU timeline analysis and bottleneck identification
- Memory usage optimization and budgeting
- Automated benchmarking for regression detection

### Production Practices

- Proper error handling and validation (Debug Layer, GPU-Based Validation, DRED)
- Telemetry and metrics collection
- Security-conscious code (bounds checking, safe integer operations)
- Professional debugging infrastructure

## Current Implementation

### Completed

- ✅ Basic ray tracing foundation (ray class, sphere intersection)
- ✅ Camera and viewport setup
- ✅ Simple CPU-based reference implementation
- ✅ PNG image output

### In Progress

- 🚧 DirectX 12 infrastructure (device, command queues, resources)
- 🚧 Compute shader ray tracing kernel
- 🚧 Performance profiling integration (PIX markers, GPU timestamps)

## Implementation Roadmap

### Phase 1: DX12 Compute Shader Foundation

Transform the CPU path tracer to use DirectX 12 compute shaders for GPU acceleration.

**Key Components:**

- DX12 device initialization and resource management
- Descriptor heap management for shader resources
- Command queue and command list infrastructure
- HLSL compute shader for ray tracing
- Memory management with proper heap types (upload, default, readback)
- Synchronization using fences and resource barriers

**Target:** GPU-accelerated path tracer achieving 10x+ performance improvement over CPU baseline, maintaining interactive framerates.

### Phase 2: Real-Time Optimizations & Benchmarking

Achieve and maintain real-time framerates with comprehensive performance measurement.

**Key Components:**

- Progressive accumulation system for quality convergence
- Temporal reprojection for camera movement
- Frame pacing and budget management
- PIX for Windows integration (GPU profiling and timeline analysis)
- GPU timestamp queries for precise timing
- Automated benchmark suite measuring FPS, frame time, and throughput
- Performance comparison: CPU vs GPU, naive vs optimized

**Target:** Consistent 30+ FPS at 1080p with progressive quality improvement for static scenes.

### Phase 3: Acceleration Structures & Advanced Optimization

Scale to complex scenes while maintaining real-time performance.

**Key Components:**

- Bounding Volume Hierarchy (BVH) construction and traversal
- GPU-based BVH updates for dynamic scenes
- Async compute for overlapping work (compute + copy operations)
- Memory streaming and resource management under budget
- Shader optimization (thread divergence reduction, wave intrinsics, occupancy tuning)
- Comparison with DXR (DirectX Raytracing) if hardware available

**Target:** Handle 10K+ primitives at 30+ FPS, demonstrating O(log n) vs O(n) performance scaling.

### Phase 4: Quality & Robustness

Build reliability and maintainability into the codebase.

**Key Components:**

- Comprehensive error handling and validation (Debug Layer, GPU-Based Validation, DRED)
- Security best practices (bounds checking, integer overflow protection, input sanitization)
- ETW (Event Tracing for Windows) provider for system-level profiling
- Automated testing (unit tests for math/intersection, integration tests for DX12 init)
- Performance regression detection
- Documentation of architecture, API, and performance characteristics

**Target:** Zero warnings, zero leaks, robust error handling, professional code quality.

### Phase 5: Advanced Features

Push beyond basics to explore cutting-edge real-time rendering techniques.

**Possible Directions:**

- **Denoising** - Spatial and temporal denoising for low-sample-count renders (Intel OIDN integration)
- **Advanced Materials** - PBR materials (metallic, roughness, glass) with importance sampling
- **Dynamic Scenes** - BVH refitting and rebuilding for animated geometry
- **Multi-GPU** - Work distribution across multiple GPUs with cross-adapter synchronization
- **Debugging Tools** - WinDbg extension for inspecting GPU structures in crash dumps
- **DXR Path** - Implement parallel DXR version for comparison with compute shader approach

**Target:** At least one advanced feature that pushes the boundaries of real-time path tracing.

## Technology Stack

### Core Technologies

- **DirectX 12** - Primary graphics API
- **HLSL** - Shader programming language
- **C++23** - Modern C++ with latest features
- **CMake** - Build system

### Libraries

- **GLM** - Mathematics library for vectors and matrices
- **stb_image_write** - Image output (PNG format)
- **vcpkg** - Dependency management

### Profiling & Debugging Tools

- **PIX for Windows** - GPU profiling and debugging
- **Windows Performance Toolkit** - System-level performance analysis (ETW/WPA)
- **WinPixEventRuntime** - PIX markers in code
- **Visual Studio Debugger** - Primary debugger
- **WinDbg** - Advanced debugging scenarios

### Future Additions

- DirectXTex (texture processing)
- D3D12 Memory Allocator (advanced memory management)
- Google Test or Catch2 (testing framework)

## Building

### Prerequisites

- Windows 10/11 (version 2004 or later)
- Visual Studio 2022 (17.8 or later)
- Windows SDK 10.0.22000.0 or later
- CMake 3.24 or later
- vcpkg (for dependency management)

### Build Instructions

```bash
# Clone the repository
git clone <repository-url>
cd pathtracer

# Configure with CMake
cmake --preset=default

# Build
cmake --build build --config Release

# Run
./build/bin/Release/pathtracer.exe
```

### Build Configurations

- **Debug** - Full debug information, all validation enabled
- **Release** - Optimized build for performance testing
- **Profile** - Release optimizations with PIX integration
- **Instrumented** - ETW provider enabled for system-level profiling

## Real-Time Performance Goals

Target metrics for a successful real-time path tracer:

| Metric                      | Target | Measured |
| :------------------------------------- | --------------- | ---------- |
| Frame Time (1080p, 1K spheres)         | < 33ms (30 FPS)            | TBD |
| Frame Time (1080p, 10K spheres w/ BVH) | < 33ms (30 FPS)            | TBD |
| Ray Throughput                         | > 1 billion rays/second    | TBD |
| Memory Usage                           | < 512MB for typical scene  | TBD |
| Startup Time                           | < 1 second to first frame  | TBD |
| Convergence Time (static scene)        | < 5 seconds to clean image | TBD |

**Hardware Target:** Modern gaming GPU (NVIDIA RTX 3060 / AMD RX 6600 or better)

**Quality vs Performance Trade-offs:**

- 1-4 samples per pixel (SPP) per frame
- Progressive accumulation when camera is static
- Denoising to maintain visual quality with low SPP
- Adaptive quality based on frame budget

## Documentation

- **ARCHITECTURE.md** _(coming soon)_ - System architecture, design decisions, and data flow
- **PERFORMANCE.md** _(coming soon)_ - Performance analysis, benchmarks, and optimization results
- **BUILD.md** _(coming soon)_ - Detailed build instructions and troubleshooting guide

## Project Structure

```
pathtracer/
├── src/                    # Source code
│   ├── core/              # DX12 device and resource management (planned)
│   ├── rendering/         # Path tracing implementation (planned)
│   ├── profiling/         # PIX and ETW integration (planned)
│   └── main.cpp           # Entry point
├── include/               # Header files
│   ├── interfaces/        # Abstract interfaces
│   ├── ray/              # Ray and camera
│   ├── shapes/           # Geometric primitives
│   └── utils/            # Utilities
├── shaders/              # HLSL compute shaders (planned)
├── tools/                # Benchmark and analysis tools (planned)
├── tests/                # Unit and integration tests (planned)
├── docs/                 # Documentation (planned)
├── scenes/               # Scene description files (planned)
├── CMakeLists.txt        # CMake configuration
├── vcpkg.json           # Dependencies
├── README.md            # This file
└── IMPLEMENTATION.md    # Detailed implementation guide
```

## Learning Resources

This project draws inspiration and knowledge from:

- [Ray Tracing in One Weekend](https://raytracing.github.io/) - Foundation for ray tracing concepts
- [Microsoft DirectX 12 Documentation](https://learn.microsoft.com/en-us/windows/win32/direct3d12/)
- [3D Game Programming with DirectX 12](http://www.d3dcoder.net/) by Frank Luna
- [Physically Based Rendering](https://www.pbr-book.org/) - Advanced rendering techniques
- [Learning DirectX 12](https://www.3dgep.com/learning-directx-12-1/) by Jeremiah van Oosten

## Contributing

This is currently a personal portfolio project and is not accepting contributions. However, feedback and suggestions are welcome via issues.

## Contributing

This is a personal learning project and I'm not accepting pull requests at this time. However, feedback, suggestions, and discussions are always welcome! Feel free to open an issue if you spot a bug, have an optimization idea, or want to discuss rendering techniques.

## License

MIT License - See LICENSE file for details

## Acknowledgments

- Peter Shirley's ["Ray Tracing in One Weekend"](https://raytracing.github.io/) series for the excellent foundation
- Microsoft for comprehensive DirectX 12 documentation and PIX profiling tools
- The graphics programming community for openly sharing knowledge and techniques
- [pbrt](https://www.pbr-book.org/) for deep insights into physically-based rendering

---

**Note:** This project is under active development and the architecture will evolve significantly as I work through the phases. Performance numbers will be updated as optimizations are implemented and measured.
