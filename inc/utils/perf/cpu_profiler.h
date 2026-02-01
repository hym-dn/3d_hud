/**
 * @file cpu_profiler.h
 * @brief CPU Performance Profiling Macros for 3D HUD Project
 * 
 * This header provides a comprehensive set of macros for CPU performance profiling
 * using the Tracy profiler. These macros enable real-time CPU execution time analysis,
 * bottleneck identification, and frame-level performance tracking.
 * 
 * @author Yameng.He
 * @version 1.0
 * @date 2025-11-28
 * @copyright Copyright (c) 2024 3D HUD Project
 */

#pragma once

#ifdef __3D_HUD_PERF_ANALYSIS_CPU__

#include "utils/utils_define.h"
#include "tracy/Tracy.hpp"

/**
 * @brief Creates a named CPU profiling zone with automatic scope management
 * 
 * This macro creates a scoped profiling zone that automatically tracks execution time
 * from the point of declaration until the end of the current scope. Ideal for measuring
 * specific code blocks or functions with custom names.
 * 
 * @param zone_name Descriptive name for the profiling zone (displayed in Tracy UI)
 * 
 * @example
 * HUD_3D_CPU_PROFILE_ZONE("GameLogicUpdate");
 * // Code to profile...
 */
#define HUD_3D_CPU_PROFILE_ZONE(zone_name) ZoneScopedN(zone_name)

/**
 * @brief Creates a profiling zone using the current function name as identifier
 * 
 * Automatically uses __FUNCTION__ macro to create a profiling zone named after
 * the containing function. Perfect for function-level performance analysis.
 * 
 * @example
 * void UpdatePhysics() {
 *     HUD_3D_CPU_PROFILE_FUNCTION();
 *     // Physics calculations...
 * }
 */
#define HUD_3D_CPU_PROFILE_FUNCTION() ZoneScoped

/**
 * @brief Creates a scoped profiling zone with custom name and automatic source info
 * 
 * Similar to HUD_3D_CPU_PROFILE_ZONE, but emphasizes the scoped nature of the
 * profiling zone. Automatically captures file, line, and function information.
 * 
 * @param zone_name Descriptive name for the profiling zone
 */
#define HUD_3D_CPU_PROFILE_SCOPED(zone_name) ZoneScopedN(zone_name)

/**
 * @brief Marks the beginning of a CPU frame in the profiler
 * 
 * Should be called at the start of each main loop iteration to delineate frame boundaries.
 * Creates a visual marker in Tracy UI for frame-based analysis. Use in conjunction with
 * HUD_3D_CPU_MARK_FRAME_END() for precise frame timing.
 * 
 * @note Frame marks are essential for correlating CPU performance with rendering frames
 */
#define HUD_3D_CPU_MARK_FRAME_START() FrameMarkStart("CPU Frame")

/**
 * @brief Marks the end of the current CPU frame in the profiler
 * 
 * Should be called at the end of each main loop iteration to complete the frame boundary
 * started by HUD_3D_CPU_MARK_FRAME_START(). Enables accurate frame time measurements
 * and FPS calculations in Tracy.
 */
#define HUD_3D_CPU_MARK_FRAME_END() FrameMarkEnd("CPU Frame")

/**
 * @brief Marks a complete frame with a single function call
 * 
 * Convenience macro that combines both frame start and end markers. Suitable for
 * simple frame-based profiling where precise frame boundaries are not required.
 * 
 * @note For detailed frame analysis, prefer using separate start/end markers
 */
#define HUD_3D_CPU_MARK_FRAME() FrameMark

/**
 * @brief Initializes the CPU profiling system with application metadata
 * 
 * Should be called during application startup to register application information
 * with the Tracy profiler. Provides context for performance analysis sessions.
 * 
 * @note Tracy profiling system initializes automatically; this provides additional metadata
 */
#define HUD_3D_CPU_INITIALIZE() TracyAppInfo("3D HUD CPU profiling system", strlen("3D HUD CPU profiling system"));

#else // __3D_HUD_PERF_ANALYSIS_CPU__

/**
 * @brief Empty macro definition when CPU profiling is disabled
 * 
 * These empty macros allow profiling code to remain in place without causing
 * performance overhead when __3D_HUD_PERF_ANALYSIS_CPU__ is not defined.
 */
#define HUD_3D_CPU_PROFILE_ZONE(zone_name)
#define HUD_3D_CPU_PROFILE_FUNCTION()
#define HUD_3D_CPU_PROFILE_SCOPED(zone_name)
#define HUD_3D_CPU_MARK_FRAME_START()
#define HUD_3D_CPU_MARK_FRAME_END()
#define HUD_3D_CPU_MARK_FRAME()
#define HUD_3D_CPU_INITIALIZE()

#endif // __3D_HUD_PERF_ANALYSIS_CPU__