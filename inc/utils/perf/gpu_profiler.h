/**
 * @file gpu_profiler.h
 * @brief GPU Performance Profiler for 3D HUD Project
 * 
 * This header defines the comprehensive GPU performance profiling interface 
 * using the Tracy profiler framework. It provides real-time GPU execution 
 * time analysis, bottleneck identification, VRAM monitoring, and graphics 
 * pipeline performance tracking for the 3D HUD application.
 * 
 * Key Features:
 * - Real-time GPU frame timing and performance metrics
 * - VRAM usage monitoring with vendor-specific optimizations
 * - Cross-platform GPU vendor detection (NVIDIA, AMD, Intel)
 * - Tracy integration for detailed performance visualization
 * - Macro-based API for easy integration into rendering code
 * 
 * @author Yameng.He
 * @version 1.0
 * @date 2025-12-04
 * @copyright Copyright (c) 2024 3D HUD Project
 */

#pragma once

#ifdef __3D_HUD_PERF_ANALYSIS_GPU__

#include <string>
#include <chrono>
#include <atomic>
#include <unordered_map>
#include "tracy/Tracy.hpp"           ///< Tracy profiling framework main header
#include "tracy/TracyC.h"            ///< Tracy C API for low-level profiling
#include "tracy/TracyOpenGL.hpp"     ///< Tracy OpenGL-specific profiling utilities
#include "utils/utils_define.h"

// GPU memory information constants for vendor-specific extensions
// These are defined only if not already provided by the OpenGL headers
#ifndef GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX
    #define GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX 0x9048  ///< NVIDIA total VRAM query
#endif
#ifndef GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX
    #define GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX 0x9049 ///< NVIDIA available VRAM query
#endif
#ifndef GL_VBO_FREE_MEMORY_ATI
    #define GL_VBO_FREE_MEMORY_ATI 0x87FB  ///< AMD VBO free memory query
#endif
#ifndef GL_TEXTURE_FREE_MEMORY_ATI
    #define GL_TEXTURE_FREE_MEMORY_ATI 0x87FC  ///< AMD texture free memory query
#endif
#ifndef GL_RENDERBUFFER_FREE_MEMORY_ATI
    #define GL_RENDERBUFFER_FREE_MEMORY_ATI 0x87FD  ///< AMD renderbuffer free memory query
#endif

namespace hud_3d
{
    namespace utils
    {
        namespace perf
        {
            /**
             * @class GpuProfiler
             * @brief Singleton class for GPU performance profiling and monitoring
             * 
             * Provides comprehensive GPU performance analysis including frame timing,
             * VRAM usage tracking, and vendor-specific optimizations. Integrates
             * with Tracy for real-time performance visualization.
             */
            class GpuProfiler
            {
            public:
                /// GPU vendor enumeration for vendor-specific optimizations
                enum class Vendor
                {
                    Unknown,    ///< Unknown or unsupported GPU vendor
                    NVIDIA,     ///< NVIDIA graphics cards
                    AMD         ///< AMD/ATI graphics cards
                };

            public:
                /// Destructor - performs cleanup of GPU profiling resources
                ~GpuProfiler() noexcept;

            public:
                /**
                 * @brief Get the singleton instance of the GPU profiler
                 * @return Reference to the singleton GpuProfiler instance
                 */
                static GpuProfiler &GetInstance() noexcept;

            public:
                /**
                 * @brief Initialize the GPU profiling system
                 * 
                 * Sets up Tracy GPU context, detects GPU vendor, and prepares
                 * for performance monitoring. Must be called before any profiling.
                 */
                void Initialize() noexcept;

                /**
                 * @brief Collect and update GPU performance metrics
                 * 
                 * Gathers current VRAM usage, frame statistics, and updates
                 * Tracy plots with real-time performance data.
                 */
                void Collect() noexcept;

            private:
                /// Private constructor for singleton pattern
                GpuProfiler() noexcept;

            private:
                // Delete copy and move operations to enforce singleton pattern
                GpuProfiler(GpuProfiler &&) = delete;
                GpuProfiler(const GpuProfiler &) = delete;
                GpuProfiler &operator=(GpuProfiler &&) = delete;
                GpuProfiler &operator=(const GpuProfiler &) = delete;

            private:
                /**
                 * @brief Update VRAM statistics using vendor-specific extensions
                 * 
                 * Queries GPU memory usage through NVIDIA or AMD extensions and
                 * updates Tracy plots with the collected data.
                 */
                void UpdateVRAMStats() noexcept;

            private:
                Vendor vendor_;  ///< Detected GPU vendor for optimization
            };
        } // namespace perf
    } // namespace utils
} // namespace hud_3d

// GPU Profiling API Macros
// These macros provide easy integration of GPU profiling into rendering code

/// Initialize the GPU profiling system (must be called once at startup)
#define HUD_3D_GPU_INITIALIZE() GpuProfiler::GetInstance().Initialize()

/// Create a GPU profiling zone with the specified name
#define HUD_3D_GPU_ZONE(name) TracyGpuZone(name)

/// Create a GPU profiling zone with custom color for visual distinction
#define HUD_3D_GPU_ZONE_COLOR(name, color) TracyGpuZoneC(name, color)

/// Collect and update GPU performance metrics (call periodically)
#define HUD_3D_GPU_COLLECT() GpuProfiler::GetInstance().Collect()

/// Mark the beginning of a GPU frame for performance analysis
#define HUD_3D_GPU_FRAME_START() TracyCFrameMarkStart("GPU Frame")

/// Mark the end of a GPU frame to complete performance measurement
#define HUD_3D_GPU_FRAME_END() TracyCFrameMarkEnd("GPU Frame")

/// Mark a complete GPU frame with a single function call
#define HUD_3D_GPU_FRAME_MARK() TracyCFrameMark

#else

#define HUD_3D_GPU_INITIALIZE()
#define HUD_3D_GPU_ZONE(name)
#define HUD_3D_GPU_ZONE_COLOR(name, color)
#define HUD_3D_GPU_COLLECT()
#define HUD_3D_GPU_FRAME_START()
#define HUD_3D_GPU_FRAME_END()
#define HUD_3D_GPU_FRAME_MARK()

#endif // __3D_HUD_PERF_ANALYSIS_GPU__