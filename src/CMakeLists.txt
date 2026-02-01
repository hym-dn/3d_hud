# =============================================================================
# 3D HUD Project - Source Directory CMake Configuration
# =============================================================================
# This CMakeLists.txt file manages the source code organization and module
# structure within the src/ directory. It serves as the entry point for
# compiling all application components and their dependencies.
#
# @author Yameng.He
# @version 1.0
# @date 2025-11-28
# @copyright Copyright (c) 2024 3D HUD Project

# =============================================================================
# CMake Version Requirements
# =============================================================================
# Inherits the minimum CMake version requirement from the root configuration
# to ensure consistency across the entire build system. This guarantees that
# all subdirectories and modules use compatible CMake features and syntax.
cmake_minimum_required(VERSION 3.16)

# =============================================================================
# Utility Module Integration
# =============================================================================
# Processes the utils/ subdirectory which contains core utility functions,
# logging infrastructure, and shared components used throughout the application.
# This module provides foundational services that other application components
# depend on for basic functionality and system integration.
add_subdirectory(utils)

# =============================================================================
# Platform Abstraction Layer Integration
# =============================================================================
# Processes the platform/ subdirectory which contains platform-specific
# implementations and abstraction interfaces. This module enables cross-platform
# compatibility by providing unified APIs for system-level operations while
# maintaining optimal performance through platform-specific optimizations.
# The platform layer handles hardware abstraction, system calls, and
# environment-specific functionality required for the 3D HUD application.
add_subdirectory(platform)

# =============================================================================
# Rendering Engine Integration
# =============================================================================
# Processes the rendering/ subdirectory which contains the core rendering engine
# components including the render pipeline, camera system, lighting, and scene
# management. This module provides the 3D graphics capabilities for the HUD system.
add_subdirectory(rendering)