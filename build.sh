#!/bin/bash
# Linux Build Script

echo "================================================"
echo "3D HUD Rendering Engine - Linux Build Script"
echo "================================================"

# Check Python
if ! command -v python3 &> /dev/null; then
    echo "Error: Python3 not found, please install Python 3.7 or higher"
    exit 1
fi

# Check if running with sudo
if [ "$EUID" -ne 0 ]; then
    echo "Info: Running with normal user privileges"
else
    echo "Warning: Running with root privileges, normal user recommended"
fi

# Run Python build script
python3 build.py "$@"

if [ $? -eq 0 ]; then
    echo ""
    echo "Build successful!"
else
    echo ""
    echo "Build failed!"
    exit 1
fi