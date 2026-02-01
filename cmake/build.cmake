# =============================================================================
# CMake Build Configuration for 3D HUD Project
# =============================================================================
# This CMake module configures compiler-specific build options and settings
# that ensure consistent compilation behavior across different platforms and
# toolchains. It addresses platform-specific requirements and optimizations.
#
# Key features:
# - Platform detection and macro definitions
# - Compiler-specific optimization flags
# - Architecture-specific configurations
# - Build type configurations
#
# @author Yameng.He
# @version 1.0
# @date 2025-11-28
# @copyright Copyright (c) 2024 3D HUD Project

# =============================================================================
# Platform Macro Definitions
# =============================================================================
# Defines platform-specific preprocessor macros that enable conditional compilation
# and platform-specific code paths. These macros help maintain clean cross-platform
# code by allowing platform-specific implementations to be selected at compile time.
#
# Supported platforms:
# - Windows (MSVC, MinGW)
# - Linux (GCC, Clang)
# - Android
# - QNX
# - macOS/iOS (Clang)
#
# Usage examples:
#   #if defined(__3D_HUD_PLATFORM_WINDOWS__)
#       // Windows-specific code
#   #elif defined(__3D_HUD_PLATFORM_LINUX__)
#       // Linux-specific code
#   #endif

# Windows platform configuration
if(WIN32)
    add_compile_definitions(__3D_HUD_PLATFORM_WINDOWS__=1)
    message(STATUS "Configuring for Windows platform")

    # Additional Windows-specific definitions
    if(MSVC)
        add_compile_definitions(__3D_HUD_COMPILER_MSVC__=1)
        add_compile_definitions(_CRT_SECURE_NO_WARNINGS)  # Disable CRT security warnings

        # UTF-8 support for MSVC to prevent C4819 warnings with Unicode characters
        add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
        add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
        message(STATUS "Enabled UTF-8 support for MSVC compiler")

        # Architecture-specific definitions based on pointer size
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            add_compile_definitions(__3D_HUD_ARCH_X64__=1)
            message(STATUS "Detected 64-bit architecture (x64)")
        else()
            add_compile_definitions(__3D_HUD_ARCH_X86__=1)
            message(STATUS "Detected 32-bit architecture (x86)")
        endif()
    endif()

    # MinGW compiler on Windows (alternative to MSVC)
    if(MINGW)
        add_compile_definitions(__3D_HUD_COMPILER_MINGW__=1)
        message(STATUS "Detected MinGW compiler on Windows")
    endif()
endif()

# Linux platform
if(UNIX AND NOT APPLE AND NOT CYGWIN)
    add_compile_definitions(__3D_HUD_PLATFORM_LINUX__=1)
    message(STATUS "Configuring for Linux platform")

    # Linux-specific compiler definitions
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        add_compile_definitions(__3D_HUD_COMPILER_GCC__=1)
        # Add -fPIC for position independent code (required for static libraries on Linux)
        add_compile_options(-fPIC)
        message(STATUS "Added -fPIC compiler flag for GCC on Linux")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_compile_definitions(__3D_HUD_COMPILER_CLANG__=1)
        # Add -fPIC for position independent code (required for static libraries on Linux)
        add_compile_options(-fPIC)
        message(STATUS "Added -fPIC compiler flag for Clang on Linux")
    endif()

    # Architecture definitions
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "amd64")
        add_compile_definitions(__3D_HUD_ARCH_X64__=1)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i386" OR CMAKE_SYSTEM_PROCESSOR MATCHES "i686")
        add_compile_definitions(__3D_HUD_ARCH_X86__=1)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
        add_compile_definitions(__3D_HUD_ARCH_ARM64__=1)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
        add_compile_definitions(__3D_HUD_ARCH_ARM__=1)
    endif()
endif()

# Android platform
if(ANDROID)
    add_compile_definitions(__3D_HUD_PLATFORM_ANDROID__=1)
    message(STATUS "Configuring for Android platform")
    
    # Add -fPIC for position independent code (required for static libraries on Android)
    add_compile_options(-fPIC)
    message(STATUS "Added -fPIC compiler flag for Android")
    
    # Android-specific architecture
    if(ANDROID_ABI MATCHES "arm64")
        add_compile_definitions(__3D_HUD_ARCH_ARM64__=1)
    elseif(ANDROID_ABI MATCHES "arm")
        add_compile_definitions(__3D_HUD_ARCH_ARM__=1)
    elseif(ANDROID_ABI MATCHES "x86_64")
        add_compile_definitions(__3D_HUD_ARCH_X64__=1)
    elseif(ANDROID_ABI MATCHES "x86")
        add_compile_definitions(__3D_HUD_ARCH_X86__=1)
    endif()
endif()

# QNX platform
if(QNX)
    add_compile_definitions(__3D_HUD_PLATFORM_QNX__=1)
    message(STATUS "Configuring for QNX platform")

    # Add -fPIC for position independent code (required for static libraries on QNX)
    add_compile_options(-fPIC)
    message(STATUS "Added -fPIC compiler flag for QNX")

    # QNX-specific architecture
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
        add_compile_definitions(__3D_HUD_ARCH_ARM64__=1)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
        add_compile_definitions(__3D_HUD_ARCH_ARM__=1)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
        add_compile_definitions(__3D_HUD_ARCH_X64__=1)
    endif()
endif()

# macOS/iOS platform
if(APPLE)
    if(IOS)
        add_compile_definitions(__3D_HUD_PLATFORM_IOS__=1)
        message(STATUS "Configuring for iOS platform")
    else()
        add_compile_definitions(__3D_HUD_PLATFORM_MACOS__=1)
        message(STATUS "Configuring for macOS platform")
    endif()
    
    # Apple platform compiler
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_compile_definitions(__3D_HUD_COMPILER_CLANG__=1)
        # Add -fPIC for position independent code (required for static libraries on Apple platforms)
        add_compile_options(-fPIC)
        message(STATUS "Added -fPIC compiler flag for Clang on Apple platforms")
    endif()
    
    # Apple platform architecture
    if(CMAKE_OSX_ARCHITECTURES MATCHES "arm64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
        add_compile_definitions(__3D_HUD_ARCH_ARM64__=1)
    elseif(CMAKE_OSX_ARCHITECTURES MATCHES "x86_64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
        add_compile_definitions(__3D_HUD_ARCH_X64__=1)
    endif()
endif()

# Build configuration macros
if(CMAKE_BUILD_TYPE MATCHES "Debug" OR CMAKE_BUILD_TYPE MATCHES "DEBUG")
    add_compile_definitions(__3D_HUD_DEBUG__=1)
    add_compile_definitions(__3D_HUD_CONFIG_DEBUG__=1)
elseif(CMAKE_BUILD_TYPE MATCHES "Release" OR CMAKE_BUILD_TYPE MATCHES "RELEASE")
    add_compile_definitions(__3D_HUD_RELEASE__=1)
    add_compile_definitions(__3D_HUD_CONFIG_RELEASE__=1)
elseif(CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
    add_compile_definitions(__3D_HUD_RELWITHDEBINFO__=1)
    add_compile_definitions(__3D_HUD_CONFIG_RELWITHDEBINFO__=1)
endif()

