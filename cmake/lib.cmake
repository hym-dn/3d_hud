# =============================================================================
# Library Configuration for 3D HUD Project
# =============================================================================
# This CMake module defines library targets and their dependencies for the
# 3D HUD system. It configures the utils library and manages optional
# dependencies based on enabled features.
#
# @author Yameng.He
# @version 1.0
# @date 2025-11-28
# @copyright Copyright (c) 2024 3D HUD Project

# Define library target name
set(UTILS_TARGET "3d_hud_utils")
set(PLATFORM_TARGET "3d_hud_platform")
set(RENDERING_TARGET "3d_hud_rendering")

# Initialize dependencies list for library
set(UTILS_TARGET_DEPENDENCIES "")
set(PLATFORM_TARGET_DEPENDENCIES "")
set(RENDERING_TARGET_DEPENDENCIES "")

# =============================================================================
# SPDLOG Dependency Configuration
# =============================================================================
# High-performance C++ logging library for general-purpose logging
# Required when SPDLOG backend is enabled in the logging system
if(SPD_LOGGER)
    if(NOT TARGET spdlog::spdlog)
        message(STATUS "Utils: Searching for SPDLOG library...")
        find_package(spdlog REQUIRED CONFIG)
        if(TARGET spdlog::spdlog)
            list(APPEND UTILS_TARGET_DEPENDENCIES spdlog::spdlog)
            message(STATUS "Utils: SPDLOG library located successfully")
        else()
            message(FATAL_ERROR "Utils: SPDLOG dependency not found - required for logging functionality")
        endif()
    else()
        list(APPEND UTILS_TARGET_DEPENDENCIES spdlog::spdlog)
        message(STATUS "Utils: SPDLOG target already configured")
    endif()
endif()

# =============================================================================
# FMT Dependency Configuration
# =============================================================================
# Fast formatting library required for string formatting operations
# Used by SPDLOG and other components for efficient text formatting
if(NOT TARGET fmt::fmt)
    message(STATUS "Utils: Searching for FMT formatting library...")
    find_package(fmt REQUIRED CONFIG)
    if(TARGET fmt::fmt)
        list(APPEND UTILS_TARGET_DEPENDENCIES fmt::fmt)
        message(STATUS "Utils: FMT library located successfully")
    else()
        message(FATAL_ERROR "Utils: FMT dependency not found - required for string formatting operations")
    endif()
else()
    list(APPEND UTILS_TARGET_DEPENDENCIES fmt::fmt)
    message(STATUS "Utils: FMT target already configured")
endif()

# =============================================================================
# SLOG2 Dependency Configuration (QNX Only)
# =============================================================================
# QNX system logging library for platform-specific logging integration
# Only available and functional on QNX/Neutrino platforms
if(S_LOGGER)
    if(CMAKE_SYSTEM_NAME STREQUAL "QNX" OR CMAKE_SYSTEM_NAME STREQUAL "Neutrino")
        # SLOG2 is part of QNX system libraries - no external dependency required
        list(APPEND UTILS_TARGET_DEPENDENCIES slog2)
        message(STATUS "Utils: SLOG2 system library configured for QNX platform")
    else()
        message(WARNING "Utils: SLOG backend is QNX-specific - functionality will be limited on non-QNX platforms")
    endif()
endif()

# =============================================================================
# External Logger Dependencies
# =============================================================================
# Placeholder for custom external logging system integration
# Allows connection to third-party logging frameworks via callback interface
if(EXTERNAL_LOGGER)
    # External logger typically requires custom dependencies
    # Add specific external logging library dependencies here when implemented
    message(STATUS "Utils: External logger integration configured")
endif()

# =============================================================================
# Tracy Performance Analysis Dependencies
# =============================================================================
# Real-time performance profiling library for CPU, memory, and GPU analysis
# Provides comprehensive performance monitoring and bottleneck identification
if(PERF_ANALYSIS_CPU OR PERF_ANALYSIS_MEMORY OR PERF_ANALYSIS_GPU)
    if(NOT TARGET Tracy::TracyClient)
        message(STATUS "tracy::tracyclient: Searching for Tracy profiling library...")
        find_package(Tracy REQUIRED CONFIG)
        if(TARGET Tracy::TracyClient)
            list(APPEND UTILS_TARGET_DEPENDENCIES Tracy::TracyClient)
            message(STATUS "find package tracy::tracyclient: Tracy profiling library located successfully")
        else()
            message(WARNING "find package tracy::tracyclient: Tracy dependency not found - performance analysis will be disabled")
        endif()
    else()
        list(APPEND UTILS_TARGET_DEPENDENCIES Tracy::TracyClient)
        message(STATUS "tracy::tracyclient: Tracy target already configured")
    endif()
endif()

# =============================================================================
# OpenGL Graphics API Dependencies
# =============================================================================
# Cross-platform graphics API for GPU rendering and compute operations
# Required for GPU profiling and any graphics-related functionality
if(PERF_ANALYSIS_GPU OR GRAPHICS_RENDERING)
    # Use CMake's built-in OpenGL find module
    find_package(OpenGL)
    
    # Find and configure glad library for OpenGL function loading
    if(NOT TARGET glad::glad)
        message(STATUS "glad::glad: Searching for glad library...")
        find_package(glad REQUIRED CONFIG)
        if(TARGET glad::glad)
            list(APPEND UTILS_TARGET_DEPENDENCIES glad::glad)
            message(STATUS "find package glad: glad library located successfully")
        else()
            message(WARNING "find package glad: glad dependency not found - OpenGL function loading may fail")
        endif()
    else()
        list(APPEND UTILS_TARGET_DEPENDENCIES glad::glad)
        message(STATUS "glad::glad: glad target already configured")
    endif()
    
    if(OpenGL_FOUND)
        # Add OpenGL library to dependencies
        list(APPEND UTILS_TARGET_DEPENDENCIES ${OPENGL_LIBRARY})
        message(STATUS "find package opengl: OpenGL library located successfully")
        
        # Platform-specific additional libraries
        if(WIN32)
            # Windows requires OpenGL32 library
            # list(APPEND PLATFORM_TARGET_DEPENDENCIES OpenGL::GL)
            # message(STATUS "Utils: Windows OpenGL32 library configured")
        elseif(APPLE)
            # macOS requires framework
            find_library(OPENGL_LIBRARY OpenGL)
            if(OPENGL_LIBRARY)
                list(APPEND UTILS_TARGET_DEPENDENCIES ${OPENGL_LIBRARY})
                message(STATUS "find library opengl: macOS OpenGL framework configured")
            endif()
        else()
            find_package(X11)
            if(X11_FOUND)
                list(APPEND UTILS_TARGET_DEPENDENCIES X11::X11)
                message(STATUS "find package x11: X11 library configured for OpenGL")
            endif()
        endif()
        
        # Check for OpenGL extensions support
        if(OpenGL_GL_FOUND)
            message(STATUS "find package opengl: OpenGL core profile support available")
        endif()
        
        if(OpenGL_GLU_FOUND)
            message(STATUS "find package opengl: OpenGL Utility library available")
        endif()
        
    else()
        message(WARNING "find package opengl: OpenGL dependency not found - GPU profiling and graphics functionality will be limited")
    endif()
endif()

# =============================================================================
# GLM Mathematics Library Configuration
# =============================================================================
# Header-only mathematics library for 3D graphics and rendering operations
# Required for vector, matrix, and quaternion operations in the rendering engine
if(NOT TARGET glm::glm)
    message(STATUS "glm::glm: Searching for GLM mathematics library...")
    find_package(glm REQUIRED CONFIG)
    if(TARGET glm::glm)
        list(APPEND PLATFORM_TARGET_DEPENDENCIES glm::glm)
        message(STATUS "find package glm: GLM mathematics library located successfully")
    else()
        message(WARNING "find package glm: GLM dependency not found - mathematical operations may fail")
    endif()
else()
    list(APPEND PLATFORM_TARGET_DEPENDENCIES glm::glm)
    message(STATUS "glm::glm: GLM target already configured")
endif()

list(APPEND PLATFORM_TARGET_DEPENDENCIES 
    ${UTILS_TARGET} 
    ${UTILS_TARGET_DEPENDENCIES})
list(APPEND RENDERING_TARGET_DEPENDENCIES 
    ${PLATFORM_TARGET} 
    ${PLATFORM_TARGET_DEPENDENCIES})

# =============================================================================
# Library Configuration Summary
# =============================================================================
message(STATUS "=== Utils Library Dependencies ===")
message(STATUS "Target Name: ${UTILS_TARGET}")
message(STATUS "Dependencies: ${UTILS_TARGET_DEPENDENCIES}")
message(STATUS "===================================")
message(STATUS "=== Platform Library Dependencies ===")
message(STATUS "Target Name: ${PLATFORM_TARGET}")
message(STATUS "Dependencies: ${PLATFORM_TARGET_DEPENDENCIES}")
message(STATUS "===================================")
message(STATUS "=== Rendering Library Dependencies ===")
message(STATUS "Target Name: ${RENDERING_TARGET}")
message(STATUS "Dependencies: ${RENDERING_TARGET_DEPENDENCIES}")
message(STATUS "===================================")