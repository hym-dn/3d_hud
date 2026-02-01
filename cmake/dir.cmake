# =============================================================================
# CMake Directory Configuration for 3D HUD Project
# =============================================================================
# This CMake module defines the project directory structure and configures
# include directories for the 3D HUD system. It ensures proper header file
# resolution across different modules and platforms.
#
# @author Yameng.He
# @version 1.0
# @date 2025-11-28
# @copyright Copyright (c) 2024 3D HUD Project

# =============================================================================
# Project Root Directory Configuration
# =============================================================================
# Defines the absolute path to the project root directory for consistent
# file path resolution across the build system. This variable serves as the
# foundation for all relative path calculations within the project.
#
# CMAKE_CURRENT_LIST_DIR points to the directory containing this file (cmake/)
# The parent directory (..) moves up one level to reach the project root
set(3D_HUD_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/..)

# =============================================================================
# Include Directory Configuration
# =============================================================================
# Configures the compiler's header search path to enable proper resolution
# of #include directives across the entire project. This ensures that source
# files can reference headers using clean, relative paths without requiring
# complex path calculations in each compilation unit.
#
# The inc/ directory contains all public header files that define the
# project's API and interface contracts between modules.
include_directories(
    ${3D_HUD_ROOT_DIR}/inc 
)

# =============================================================================
# Configuration Verification Output
# =============================================================================
# Provides diagnostic output during CMake configuration to verify that the
# project directory structure has been correctly detected and configured.
# This helps developers quickly identify path resolution issues and ensures
# that all subsequent build operations reference the correct locations.
message("3D_HUD_ROOT_DIR: " ${3D_HUD_ROOT_DIR})