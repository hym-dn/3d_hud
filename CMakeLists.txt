# =============================================================================
# 3D HUD Project - Main CMake Configuration File
# =============================================================================
# This is the root CMakeLists.txt file that orchestrates the entire build system
# for the 3D HUD project. It defines the project structure, minimum CMake version
# requirements, and includes all necessary configuration modules.
#
# @author Yameng.He
# @version 1.0
# @date 2025-11-28
# @copyright Copyright (c) 2024 3D HUD Project

# =============================================================================
# CMake Version Requirements
# =============================================================================
# Ensures that the build system requires at least CMake 3.16, which provides
# modern features and compatibility with the project's build configurations.
# This version supports generator expressions, improved dependency management,
# and enhanced cross-platform compilation capabilities.
cmake_minimum_required(VERSION 3.16)

# =============================================================================
# Project Definition
# =============================================================================
# Defines the main project name and establishes the foundation for the build
# system. The project name "3d_hud" is used throughout the CMake configuration
# for target naming, dependency resolution, and build artifact identification.
project(3d_hud)

# =============================================================================
# Build Configuration Modules
# =============================================================================
# Includes specialized CMake modules that configure different aspects of the
# build system. Each module handles a specific domain of configuration to
# maintain modularity and separation of concerns.

# Configuration options and feature flags for the entire project
# Defines build-time switches for logging, performance analysis, and other
# configurable features that can be enabled or disabled based on requirements.
include(cmake/option.cmake)

# Directory structure and path configuration
# Establishes the project's directory hierarchy and configures include paths
# to ensure proper header file resolution across all source modules.
include(cmake/dir.cmake)

# Compiler-specific build settings and optimizations
# Configures platform-specific compiler flags, encoding settings, and other
# build-time optimizations that ensure consistent compilation behavior.
include(cmake/build.cmake)

# Library dependency management and target configuration
# Manages third-party library dependencies, defines library targets, and
# configures the linkage between project components and external libraries.
include(cmake/lib.cmake)

# =============================================================================
# Source Code Integration
# =============================================================================
# Processes the main source code directory, which contains all the application
# logic, rendering components, and system modules. This directive triggers
# the compilation of all source files within the src/ directory hierarchy.
add_subdirectory(src)