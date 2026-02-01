# =============================================================================
# 3D HUD Project Configuration Options
# =============================================================================
# This file defines build-time configuration options for the 3D HUD system.
# Options control feature selection, platform-specific behavior, and 
# third-party library integration.
#
# @author Yameng.He
# @version 1.0
# @date 2025-11-28
# @copyright Copyright (c) 2024 3D HUD Project

# =============================================================================
# Logging System Configuration
# =============================================================================
# Configure logging backend selection and compilation definitions
# Multiple logging backends can be enabled simultaneously for different platforms
# Backends provide thread-safe logging with different performance characteristics

# SPDLOG: High-performance C++ logging library (recommended for general use)
option(SPD_LOGGER "Enable SPDLOG logging backend for general-purpose logging" ON)

# SLOG: QNX-specific system logging backend (platform-dependent)
# Only effective when building for QNX/Neutrino platforms
option(S_LOGGER "Enable SLOG logging backend for QNX platform integration" OFF)

# External logger: Custom logging interface for third-party integration
# Enables callback-based logging to external systems or frameworks
option(EXTERNAL_LOGGER "Enable external logger integration via callback interface" OFF)

# Configure logging compilation definitions and validation
if (SPD_LOGGER)
    add_compile_definitions(__3D_HUD_SPD_LOGGER__=1)
    message(STATUS "SPDLOG logging backend enabled")
endif()

if (S_LOGGER)
    add_compile_definitions(-D__3D_HUD_S_LOGGER__=1)
    # Validate platform compatibility for SLOG backend
    if(NOT CMAKE_SYSTEM_NAME STREQUAL "QNX" AND NOT CMAKE_SYSTEM_NAME STREQUAL "Neutrino")
        message(WARNING "SLOG backend is QNX-specific - functionality may be limited")
    endif()
    message(STATUS "SLOG logging backend enabled for QNX platform")
endif()

if (EXTERNAL_LOGGER)
    add_compile_definitions(-D__3D_HUD_EXTERNAL_LOGGER__=1)
    message(STATUS "External logger integration enabled")
endif()

# Validate at least one logging backend is enabled
if(NOT SPD_LOGGER AND NOT S_LOGGER AND NOT EXTERNAL_LOGGER)
    message(WARNING "No logging backend enabled - logging functionality will be disabled")
endif()

# =============================================================================
# Graphics API Configuration
# =============================================================================
# Graphics API selection for rendering backend
# Only one API can be selected at compile time for optimal performance
# Each API has different platform support and feature sets
set(HUD_3D_GRAPHICS_API "OPENGL" CACHE STRING "Graphics API selection (OPENGL, VULKAN, DIRECT3D, METAL)")
set_property(CACHE HUD_3D_GRAPHICS_API PROPERTY STRINGS OPENGL VULKAN DIRECT3D METAL)

# Render Engine Interface Definitions
# These define the backend interface architecture
add_compile_definitions(__3D_HUD_RENDER_ENGINE_INTERFACE__=1)

# Validate and configure selected graphics API
if(HUD_3D_GRAPHICS_API STREQUAL "OPENGL")
    add_compile_definitions(__3D_HUD_GRAPHICS_API_OPENGL__=1)
    message(STATUS "Graphics API: OpenGL selected")
    # Platform-specific OpenGL configuration
    if(WIN32)
        add_compile_definitions(__3D_HUD_OPENGL_WGL__=1)
        message(STATUS "  - Using WGL (Windows OpenGL)")
    elseif(UNIX AND NOT APPLE)
        add_compile_definitions(__3D_HUD_OPENGL_GLX__=1)
        message(STATUS "  - Using GLX (Linux OpenGL)")
    elseif(ANDROID)
        add_compile_definitions(__3D_HUD_OPENGL_ES__=1)
        add_compile_definitions(__3D_HUD_OPENGL_EGL__=1)
        message(STATUS "  - Using EGL (Android OpenGL ES)")
    elseif(QNX)
        add_compile_definitions(__3D_HUD_OPENGL_ES__=1)
        add_compile_definitions(__3D_HUD_OPENGL_EGL__=1)
        message(STATUS "  - Using EGL (QNX OpenGL ES)")
    endif()
elseif(HUD_3D_GRAPHICS_API STREQUAL "VULKAN")
    add_compile_definitions(__3D_HUD_GRAPHICS_API_VULKAN__=1)
    message(STATUS "Graphics API: Vulkan selected")
elseif(HUD_3D_GRAPHICS_API STREQUAL "DIRECT3D")
    add_compile_definitions(__3D_HUD_GRAPHICS_API_DIRECT3D__=1)
    message(STATUS "Graphics API: Direct3D selected")
    # Windows-only validation
    if(NOT WIN32)
        message(FATAL_ERROR "Direct3D is only supported on Windows platforms")
    endif()
elseif(HUD_3D_GRAPHICS_API STREQUAL "METAL")
    add_compile_definitions(__3D_HUD_GRAPHICS_API_METAL__=1)
    message(STATUS "Graphics API: Metal selected")
    # Apple-only validation
    if(NOT APPLE)
        message(FATAL_ERROR "Metal is only supported on Apple platforms")
    endif()
else()
    message(FATAL_ERROR "Invalid graphics API selected: ${HUD_3D_GRAPHICS_API}")
endif()

# =============================================================================
# Performance Analysis Configuration
# =============================================================================
# Tracy-based performance profiling system for CPU, memory, and GPU analysis
# Provides real-time performance monitoring and bottleneck identification
# Granular control over profiling subsystems for optimization flexibility
option(PERF_ANALYSIS_CPU "Enable CPU execution time profiling for performance optimization" ON)
option(PERF_ANALYSIS_MEMORY "Enable memory allocation and usage tracking for leak detection" OFF) 
option(PERF_ANALYSIS_GPU "Enable GPU rendering performance analysis for graphics optimization" ON)
option(MEMORY_MONITOR "Enable memory allocation and usage tracking for leak detection" ON)

# Configure CPU profiling subsystem with low-overhead instrumentation
if(PERF_ANALYSIS_CPU)
    add_compile_definitions(__3D_HUD_PERF_ANALYSIS_CPU__=1)
    message(STATUS "CPU execution profiling subsystem enabled")
endif()

# Configure memory profiling subsystem for allocation tracking
if(PERF_ANALYSIS_MEMORY)
    add_compile_definitions(__3D_HUD_PERF_ANALYSIS_MEMORY__=1)
    message(STATUS "Memory allocation tracking subsystem enabled")
endif()
if(MEMORY_MONITOR)
    add_compile_definitions(__3D_HUD_MEMORY_MONITOR__=1)
    message(STATUS "Memory allocation tracking subsystem enabled")
endif()

# Configure GPU profiling subsystem for rendering performance analysis
if(PERF_ANALYSIS_GPU)
    add_compile_definitions(__3D_HUD_PERF_ANALYSIS_GPU__=1)
    message(STATUS "GPU rendering performance subsystem enabled")
endif()

# =============================================================================
# Configuration Summary
# =============================================================================
message(STATUS "=== 3D HUD Configuration Summary ===")
message(STATUS "Graphics API: ${HUD_3D_GRAPHICS_API}")
message(STATUS "Logging Backends: SPDLOG=${SPD_LOGGER}, SLOG=${S_LOGGER}, External=${EXTERNAL_LOGGER}")
message(STATUS "Performance Analysis: CPU=${PERF_ANALYSIS_CPU}, Memory=${PERF_ANALYSIS_MEMORY}, GPU=${PERF_ANALYSIS_GPU}")
message(STATUS "=====================================")