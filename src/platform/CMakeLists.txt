# =============================================================================
# 3D HUD Project - Platform Abstraction Layer CMake Configuration
# =============================================================================
# This CMakeLists.txt file configures the platform abstraction library, which
# provides cross-platform interfaces and implementations for system-specific
# functionality. The module dynamically selects appropriate source files based
# on the target platform to ensure optimal performance and compatibility.
#
# @author Yameng.He
# @version 1.0
# @date 2025-12-13
# @copyright Copyright (c) 2024 3D HUD Project

# =============================================================================
# CMake Version Requirements
# =============================================================================
# Inherits the minimum CMake version requirement from the root configuration
# to maintain consistency across all build system components. This ensures
# compatibility with modern CMake features and syntax patterns.
cmake_minimum_required(VERSION 3.16)

# =============================================================================
# Platform Library Project Definition
# =============================================================================
# Defines the platform abstraction library project using the target name
# specified by the parent configuration. This allows for flexible naming
# conventions and facilitates integration with the overall build system.
project(${PLATFORM_TARGET})

# =============================================================================
# Platform-Specific Source File Selection
# =============================================================================
# Initializes an empty list for platform-specific source directories and
# conditionally adds Windows-specific source files when building on Windows.
# This approach enables selective compilation of platform-specific code
# while maintaining a clean cross-platform architecture.
set(PLATFORM_SOURCE_DIRS "./*.cpp")
if (WIN32)
    # Include Windows platform implementation files
    list(APPEND PLATFORM_SOURCE_DIRS windows/*.cpp)
endif()

# =============================================================================
# Source File Discovery
# =============================================================================
# Recursively gathers all platform implementation source files using glob
# pattern matching. This automates the process of including new platform
# implementations without requiring manual file list maintenance.
file(GLOB_RECURSE PLATFORM_SOURCES
    ${PLATFORM_SOURCE_DIRS}
)

# =============================================================================
# Static Library Creation
# =============================================================================
# Creates a static library containing all platform abstraction layer
# implementations. This library provides the necessary interfaces for
# system-specific functionality while maintaining clean separation of
# concerns and facilitating dependency management.
add_library(${PLATFORM_TARGET} STATIC ${PLATFORM_SOURCES})

# =============================================================================
# Dependency Linking
# =============================================================================
# Links the platform library with its required dependencies, ensuring
# that all necessary system libraries and third-party components are
# properly integrated into the final build artifact.
target_link_libraries(${PLATFORM_TARGET} PRIVATE 
    ${UTILS_TARGET_DEPENDENCIES}
    ${PLATFORM_TARGET_DEPENDENCIES})

# =============================================================================
# Build Status Reporting
# =============================================================================
# Provides detailed status messages during the configuration phase to
# facilitate debugging and verification of the platform layer setup.
message(STATUS "Platform Library Configuration Complete")
message(STATUS "  Target: ${PLATFORM_TARGET}")
message(STATUS "  Source Files: ${PLATFORM_SOURCES}")
message(STATUS "  Dependencies: ${PLATFORM_TARGET_DEPENDENCIES}")