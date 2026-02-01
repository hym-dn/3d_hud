#!/usr/bin/env python3
"""
@file build.py
@brief 3D HUD Rendering Engine Cross-Platform Build Script

This script provides cross-platform build capabilities for the 3D HUD Rendering Engine,
supporting Windows, Linux, Android, and QNX platforms. It uses Conan for dependency
management and CMake for build configuration.

@details The script automatically detects the current platform and architecture,
installs dependencies using Conan, configures the project with CMake, and builds
the project. It supports multiple build types and architectures.

@note Requires Python 3.7+, Conan 2.x, and CMake 3.16+

@author Yameng.He
@version 1.0.0
@date 2025-12-02
"""

import os
import sys
import platform
import subprocess
import argparse
from pathlib import Path

def detect_platform():
    """
    @brief Detect the current operating system platform.

    @return str The detected platform name ('windows', 'linux', 'macos', or 'unknown')
    """
    system = platform.system().lower()
    if system == "windows":
        return "windows"
    elif system == "linux":
        return "linux"
    elif system == "darwin":
        return "macos"
    else:
        return "unknown"

def check_android_prerequisites():
    """
    @brief Check if Android build prerequisites are met.

    @details Verifies that ANDROID_NDK_HOME environment variable is set
    and the specified NDK path exists.

    @return bool True if prerequisites are met, False otherwise
    """
    android_ndk_home = os.environ.get("ANDROID_NDK_HOME")
    if not android_ndk_home:
        print("Error: Please set ANDROID_NDK_HOME environment variable")
        return False

    if not os.path.exists(android_ndk_home):
        print("Error: ANDROID_NDK_HOME path does not exist")
        return False

    print(f"Android NDK path: {android_ndk_home}")
    return True

def check_qnx_prerequisites():
    """
    @brief Check if QNX build prerequisites are met.

    @details Verifies that QNX_SDP_HOME environment variable is set
    and the specified QNX SDP path exists.

    @return bool True if prerequisites are met, False otherwise
    """
    qnx_sdp_home = os.environ.get("QNX_SDP_HOME")
    if not qnx_sdp_home:
        print("Error: Please set QNX_SDP_HOME environment variable")
        return False

    if not os.path.exists(qnx_sdp_home):
        print("Error: QNX_SDP_HOME path does not exist")
        return False

    print(f"QNX SDP path: {qnx_sdp_home}")
    return True

def check_prerequisites(target_platform=None):
    """
    @brief Check if build prerequisites are satisfied.

    @details Verifies the availability of required tools (Conan, CMake)
    and platform-specific prerequisites (Android NDK, QNX SDP).

    @param target_platform Optional[str] Target platform to check ('android', 'qnx', etc.)
    @return bool True if all prerequisites are met, False otherwise
    """
    print("Checking build prerequisites...")

    # Check Conan
    try:
        result = subprocess.run(["conan", "--version"], capture_output=True, text=True)
        if result.returncode != 0:
            print("Error: Conan not installed or not in PATH")
            return False
        print(f"Conan version: {result.stdout.strip()}")
    except FileNotFoundError:
        print("Error: Conan not installed")
        return False

    # Check CMake
    try:
        result = subprocess.run(["cmake", "--version"], capture_output=True, text=True)
        if result.returncode != 0:
            print("Error: CMake not installed or not in PATH")
            return False
        print(f"CMake version: {result.stdout.split(' ')[2].strip()}")
    except FileNotFoundError:
        print("Error: CMake not installed")
        return False

    # Platform-specific checks
    if target_platform == "android":
        if not check_android_prerequisites():
            return False
    elif target_platform == "qnx":
        if not check_qnx_prerequisites():
            return False

    print("Prerequisites check completed")
    return True

def build_for_windows(project_root, conan_dir, build_dir, build_type, arch=None, verbose=False):
    """
    @brief Build the project for Windows platform.

    @details Installs dependencies using Conan, configures the project with CMake
    using Visual Studio 2022 generator, and builds the project.

    @param project_root Path Root directory of the project
    @param conan_dir Path Directory for Conan-generated files
    @param build_dir Path Directory for build output
    @param build_type str Build type ('Debug', 'Release', 'RelWithDebInfo')
    @param arch Optional[str] Target architecture ('x86', 'x86_64', 'armv8')
    @param verbose bool Enable verbose output
    @return bool True if build succeeded, False otherwise
    """
    print(f"Building Windows version... {conan_dir}")

    if not arch:
        arch = "x86_64"

    # Install dependencies using conanfile.txt
    cmd = [
        "conan",
        "install",
        ".",
        "--output-folder",
        str(conan_dir),
        "--build",
        "missing",
        "-s",
        "os=Windows",
        "-s",
        f"arch={arch}",
        "-s",
        "compiler=msvc",
        "-s",
        "compiler.version=195",
        "-s",
        f"compiler.cppstd=17"
    ]
    if verbose:
        cmd.append("--verbose")

    result = subprocess.run(cmd, cwd=project_root)
    if result.returncode != 0:
        print("Error: Conan installation failed")
        return False

    # Configure CMake project
    cmd = [
        "cmake",
        "-S",
        ".",
        "-B",
        "build",
        "-G",
        "Visual Studio 18 2026",
        f"-DCMAKE_TOOLCHAIN_FILE={conan_dir}/build/generators/conan_toolchain.cmake",
        f"-DCMAKE_PREFIX_PATH={conan_dir}",
        f"-DCMAKE_BUILD_TYPE={build_type}"
    ]
    if arch == "x86":
        cmd.extend(["-A", "Win32"])
    elif arch == "x86_64":
        cmd.extend(["-A", "x64"])
    elif arch == "armv8":
        cmd.extend(["-A", "ARM64"])

    if verbose:
        cmd.append("--verbose")

    result = subprocess.run(cmd, cwd=project_root)
    if result.returncode != 0:
        print("Error: CMake configuration failed")
        return False

    # Build project
    cmd = ["cmake", "--build", "build", "--config", build_type]
    if verbose:
        cmd.append("--verbose")

    result = subprocess.run(cmd, cwd=project_root)
    if result.returncode != 0:
        print("Error: Build failed")
        return False

    return True

def build_for_linux(project_root, conan_dir, build_dir, build_type, arch=None, verbose=False):
    """
    @brief Build the project for Linux platform.

    @details Installs dependencies using Conan, configures the project with CMake
    using Unix Makefiles generator, and builds the project with parallel compilation.

    @param project_root Path Root directory of the project
    @param conan_dir Path Directory for Conan-generated files
    @param build_dir Path Directory for build output
    @param build_type str Build type ('Debug', 'Release', 'RelWithDebInfo')
    @param arch Optional[str] Target architecture ('x86', 'x86_64', 'armv7', 'armv8')
    @param verbose bool Enable verbose output
    @return bool True if build succeeded, False otherwise
    """
    print("Building Linux version...")

    if not arch:
        arch = "x86_64"

    # Install dependencies using conanfile.txt
    cmd = [
        "conan",
        "install",
        ".",
        "--output-folder",
        str(conan_dir),
        "--build",
        "missing",
        "-s",
        "os=Linux",
        "-s",
        f"arch={arch}",
        "-s",
        "compiler=gcc",
        "-s",
        "compiler.version=11",
        "-s",
        f"compiler.cppstd=17",
    ]
    if verbose:
        cmd.append("--verbose")

    result = subprocess.run(cmd, cwd=project_root)
    if result.returncode != 0:
        print("Error: Conan installation failed")
        return False

    # Configure CMake project
    cmd = [
        "cmake",
        "-S",
        ".",
        "-B",
        "build",
        f"-DCMAKE_BUILD_TYPE={build_type}",
        f"-DCMAKE_PREFIX_PATH={conan_dir}",
        f"-DCMAKE_TOOLCHAIN_FILE={conan_dir}/build/generators/conan_toolchain.cmake"
    ]

    # Set cross-compilation toolchain (if non-x86_64 architecture is specified)
    if arch != "x86_64":
        if arch == "x86":
            cmd.extend(["-DCMAKE_C_FLAGS=-m32", "-DCMAKE_CXX_FLAGS=-m32"])
        elif arch == "armv7":
            cmd.extend(["-DCMAKE_TOOLCHAIN_FILE=cmake/arm-linux-gnueabihf.cmake"])
        elif arch == "armv8":
            cmd.extend(["-DCMAKE_TOOLCHAIN_FILE=cmake/aarch64-linux-gnu.cmake"])

    if verbose:
        cmd.append("--verbose")

    result = subprocess.run(cmd, cwd=project_root)
    if result.returncode != 0:
        print("Error: CMake configuration failed")
        return False

    # Build project
    import multiprocessing
    jobs = multiprocessing.cpu_count()
    cmd = ["cmake", "--build", "build", "-j", str(jobs)]
    if verbose:
        cmd.append("--verbose")

    result = subprocess.run(cmd, cwd=project_root)
    if result.returncode != 0:
        print("Error: Build failed")
        return False

    return True

def build_for_android(project_root, conan_dir, build_dir, build_type, arch=None, verbose=False):
    """
    @brief Build the project for Android platform.

    @details Installs dependencies using Conan with Android settings, configures the project
    with CMake using Android NDK toolchain, and builds the project for the specified ABI.

    @param project_root Path Root directory of the project
    @param conan_dir Path Directory for Conan-generated files
    @param build_dir Path Directory for build output
    @param build_type str Build type ('Debug', 'Release', 'RelWithDebInfo')
    @param arch Optional[str] Target architecture ('armv7', 'armv8', 'x86', 'x86_64')
    @param verbose bool Enable verbose output
    @return bool True if build succeeded, False otherwise
    """
    print("Building Android version...")

    android_ndk_home = os.environ.get("ANDROID_NDK_HOME")
    android_build_dir = build_dir / "android"
    android_build_dir.mkdir(exist_ok=True)

    if not arch:
        arch = "armv8"

    # Map architecture to Android ABI
    arch_to_abi = {
        "armv7": "armeabi-v7a",
        "armv8": "arm64-v8a",
        "x86": "x86",
        "x86_64": "x86_64",
    }

    if arch not in arch_to_abi:
        print(f"Error: Unsupported Android architecture: {arch}")
        return False

    android_abi = arch_to_abi[arch]

    # Install dependencies using conanfile.txt
    cmd = [
        "conan",
        "install",
        ".",
        "--output-folder",
        str(conan_dir),
        "--build",
        "missing",
        "-s",
        "os=Android",
        "-s",
        f"arch={arch}",
        "-s",
        "compiler=clang",
        "-s",
        "compiler.version=12",
        "-s",
        f"compiler.cppstd=17",
    ]
    if verbose:
        cmd.append("--verbose")

    result = subprocess.run(cmd, cwd=project_root)
    if result.returncode != 0:
        print("Error: Conan installation failed")
        return False

    # Configure Android project
    cmd = [
        "cmake",
        "-S",
        ".",
        "-B",
        "build/android",
        f"-DCMAKE_TOOLCHAIN_FILE={android_ndk_home}/build/cmake/android.toolchain.cmake",
        f"-DANDROID_ABI={android_abi}",
        "-DANDROID_PLATFORM=android-24",
        f"-DCMAKE_BUILD_TYPE={build_type}",
        "-DHUD_ENGINE_PLATFORM_ANDROID=ON",
        "-DHUD_ENGINE_USE_EGL=ON",
        f"-DCMAKE_PREFIX_PATH={conan_dir}",
        f"-DCMAKE_TOOLCHAIN_FILE={conan_dir}/build/generators/conan_toolchain.cmake"
    ]
    if verbose:
        cmd.append("--verbose")

    result = subprocess.run(cmd, cwd=project_root)
    if result.returncode != 0:
        print("Error: Android CMake configuration failed")
        return False

    # Build Android project
    cmd = ["cmake", "--build", "build/android", "-j", "4"]
    if verbose:
        cmd.append("--verbose")

    result = subprocess.run(cmd, cwd=project_root)
    if result.returncode != 0:
        print("Error: Android build failed")
        return False

    return True

def build_for_qnx(project_root, conan_dir, build_dir, build_type, arch=None, verbose=False):
    """QNX平台构建"""
    print("构建QNX版本...")

    qnx_sdp_home = os.environ.get("QNX_SDP_HOME")
    qnx_build_dir = build_dir / "qnx"
    qnx_build_dir.mkdir(exist_ok=True)

    if not arch:
        arch = "armv7"

    # Install dependencies using conanfile.txt
    cmd = [
        "conan",
        "install",
        ".",
        "--output-folder",
        str(conan_dir),
        "--build",
        "missing",
        "-s",
        "os=Neutrino",
        "-s",
        f"arch={arch}",
        "-s",
        "compiler=qcc",
        "-s",
        "compiler.version=5.4",
        "-s",
        f"compiler.cppstd=17",
    ]
    if verbose:
        cmd.append("--verbose")

    result = subprocess.run(cmd, cwd=project_root)
    if result.returncode != 0:
        print("Error: Conan installation failed")
        return False

    # Configure QNX project
    cmd = [
        "cmake",
        "-S",
        ".",
        "-B",
        "build/qnx",
        f"-DCMAKE_TOOLCHAIN_FILE={qnx_sdp_home}/qnx710/cmake/toolchain.cmake",
        f"-DCMAKE_BUILD_TYPE={build_type}",
        "-DHUD_ENGINE_PLATFORM_QNX=ON",
        "-DHUD_ENGINE_USE_EGL=ON",
        f"-DCMAKE_PREFIX_PATH={conan_dir}",
        f"-DCMAKE_TOOLCHAIN_FILE={conan_dir}/build/generators/conan_toolchain.cmake"
    ]

    # Set QNX architecture specific configuration
    if arch == "x86":
        cmd.extend(["-DQNX_TARGET_CPU=x86"])
    elif arch == "armv7":
        cmd.extend(["-DQNX_TARGET_CPU=armv7"])
    elif arch == "armv8":
        cmd.extend(["-DQNX_TARGET_CPU=aarch64le"])

    if verbose:
        cmd.append("--verbose")

    result = subprocess.run(cmd, cwd=project_root)
    if result.returncode != 0:
        print("Error: QNX CMake configuration failed")
        return False

    # Build QNX project
    cmd = ["cmake", "--build", "build/qnx", "-j", "4"]
    if verbose:
        cmd.append("--verbose")

    result = subprocess.run(cmd, cwd=project_root)
    if result.returncode != 0:
        print("Error: QNX build failed")
        return False

    return True

def build_project(target_platform=None, arch=None, clean=False, verbose=False, build_type="Release"):
    """
    @brief Main build function that orchestrates the entire build process.

    @details This is the main entry point for building the project. It handles platform
    detection, directory setup, dependency installation, and delegates to platform-specific
    build functions.

    @param target_platform Optional[str] Target platform ('windows', 'linux', 'android', 'qnx')
    @param arch Optional[str] Target architecture
    @param clean bool Clean build directories before building
    @param verbose bool Enable verbose output
    @param build_type str Build type ('Debug', 'Release', 'RelWithDebInfo')
    @return bool True if build succeeded, False otherwise
    """
    project_root = Path(__file__).parent.absolute()
    build_dir = project_root / "build"
    conan_dir = project_root / "conan"

    if not target_platform:
        target_platform = detect_platform()

    if not arch:
        if target_platform == "windows":
            arch = "x86_64"
        elif target_platform == "linux":
            arch = "x86_64"
        elif target_platform == "android":
            arch = "armv8"
        elif target_platform == "qnx":
            arch = "armv7"

    print("=" * 60)
    print("3D HUD Rendering Engine - Cross-Platform Build Script")
    print("=" * 60)
    print(f"Target platform: {target_platform}")
    print(f"Target architecture: {arch}")
    print(f"Build type: {build_type}")
    print(f"Build directory: {build_dir}")
    print(f"Conan directory: {conan_dir}")
    print("-" * 60)

    # Clean old build
    if clean:
        import shutil
        if build_dir.exists():
            shutil.rmtree(build_dir)
            print("Cleaned old build directory")
        if conan_dir.exists():
            shutil.rmtree(conan_dir)
            print("Cleaned old Conan directory")
    
    build_dir.mkdir(exist_ok=True)
    conan_dir.mkdir(exist_ok=True)

    # Platform-specific build
    if target_platform == "windows":
        success = build_for_windows(
            project_root, conan_dir, build_dir, build_type, arch, verbose
        )
    elif target_platform == "linux":
        success = build_for_linux(
            project_root, conan_dir, build_dir, build_type, arch, verbose
        )
    elif target_platform == "android":
        success = build_for_android(
            project_root, conan_dir, build_dir, build_type, arch, verbose
        )
    elif target_platform == "qnx":
        success = build_for_qnx(
            project_root, conan_dir, build_dir, build_type, arch, verbose
        )
    else:
        print(f"Error: Unsupported platform: {target_platform}")
        return False

    if success:
        print("\n" + "=" * 60)
        print("Build completed successfully!")
        print("=" * 60)

    return success

def main():
    """
    @brief Main entry point of the build script.

    @details Parses command line arguments, validates prerequisites,
    and initiates the build process.

    @return int Exit code (0 for success, 1 for failure)
    """
    parser = argparse.ArgumentParser(
        description="3D HUD Rendering Engine Cross-Platform Build Script"
    )

    parser.add_argument(
        "--platform",
        choices=["windows", "linux", "android", "qnx"],
        help="Target platform (default: auto-detect)",
    )
    parser.add_argument(
        "--arch",
        choices=["x86", "x86_64", "armv7", "armv8"],
        help="Target architecture (default: auto-selected based on platform)",
    )
    parser.add_argument(
        "--build-type",
        choices=["Debug", "Release", "RelWithDebInfo"],
        default="Release",
        help="Build type",
    )
    parser.add_argument("--verbose", "-v", action="store_true", help="Verbose output")
    parser.add_argument("--clean", action="store_true", help="Clean build")

    args = parser.parse_args()

    if not check_prerequisites(args.platform):
        sys.exit(1)

    success = build_project(
        target_platform=args.platform,
        arch=args.arch,
        clean=args.clean,
        verbose=args.verbose,
        build_type=args.build_type,
    )

    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()