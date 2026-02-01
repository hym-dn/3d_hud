# =============================================================================
# Utils Module CMake Configuration for 3D HUD Project
# =============================================================================
# This CMake file configures the utils module which provides core utility
# components including logging infrastructure, string manipulation utilities,
# and other foundational functionality used throughout the 3D HUD system.
# The module serves as the central repository for shared code and services.
#
# @author Yameng.He
# @version 1.0
# @date 2025-11-28
# @copyright Copyright (c) 2024 3D HUD Project

# =============================================================================
# CMake Version Requirements
# =============================================================================
# Inherits the minimum CMake version requirement to maintain consistency
# with the overall project build system and ensure compatibility with
# modern CMake features used throughout the configuration.
cmake_minimum_required(VERSION 3.16)

# =============================================================================
# Project Definition
# =============================================================================
# Defines the utility library target using the pre-configured target name
# from the lib.cmake module. This ensures consistent naming and dependency
# resolution across the entire build system.
project(${UTILS_TARGET})

# =============================================================================
# Source Files Collection
# =============================================================================
# Collects all source files from the utils module using recursive globbing.
# This approach automatically discovers new source files without requiring
# manual updates to the CMake configuration when adding new utility components.
#
# Note: GLOB_RECURSE is used for convenience during development, but in
# production builds, explicit file listing is recommended for better
# incremental build performance and deterministic behavior.
file(GLOB_RECURSE UTILS_SOURCES
    "log/*.cpp"
    "log/*.h"
    "string/*.cpp"
    "string/*.h"
    "perf/*.cpp"
    "perf/*.h"
    "memory/*.c"
    "memory/*.cpp"
    "memory/*.h"
)

# =============================================================================
# Library Target Creation
# =============================================================================
# Creates a static library target for the utility components. Static linking
# is preferred for utility libraries to avoid runtime dependency issues and
# ensure self-contained application deployment.
#
# The library includes both implementation (.cpp) and interface (.h) files
# to support proper IDE integration and development tooling.
add_library(${UTILS_TARGET} STATIC ${UTILS_SOURCES})

# =============================================================================
# Dependency Management
# =============================================================================
# Links the utility library with its required dependencies that were
# configured in the lib.cmake module. This includes logging libraries,
# formatting utilities, and performance analysis tools.
#
# The PRIVATE linkage scope ensures that dependencies are not propagated
# to consumers of the utility library, maintaining clean interface boundaries.
target_link_libraries(${UTILS_TARGET} PRIVATE ${UTILS_TARGET_DEPENDENCIES})

if(MSVC)
    set_source_files_properties(memory/*.c PROPERTIES LANGUAGE C)
    target_compile_definitions(${UTILS_TARGET} PRIVATE 
        RPMALLOC_COMPILE=1
        _UNICODE
        UNICODE
        _CRT_SECURE_NO_WARNINGS
    )
    target_compile_options(${UTILS_TARGET} PRIVATE 
        /std:c11
        /experimental:c11atomics
        /Zi /Oi /Oy- /GS- /Gy- /Qpar- /fp:fast /fp:except- 
        /Zc:forScope /Zc:wchar_t /GR- /W4 /wd4201
    )
    set_target_properties(${UTILS_TARGET} PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
    )
    target_link_libraries(${UTILS_TARGET} PUBLIC kernel32.lib user32.lib shell32.lib advapi32.lib)
else()
    target_compile_options(${UTILS_TARGET} PRIVATE -std=c11 -O3)
endif()

# =============================================================================
# Build Configuration Summary
# =============================================================================
# Provides diagnostic output to verify the utility library configuration
# and help developers understand the module's structure and dependencies.
message(STATUS "Utils Library Configuration Complete")
message(STATUS "  Target: ${UTILS_TARGET}")
message(STATUS "  Source Files: ${UTILS_SOURCES}")
message(STATUS "  Dependencies: ${UTILS_TARGET_DEPENDENCIES}")