@echo off
chcp 65001 >nul
REM Windows Build Script

echo ================================================
echo 3D HUD Rendering Engine - Windows Build Script
echo ================================================

REM Check Python
python --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: Python not found, please install Python 3.7 or higher
    pause
    exit /b 1
)

REM Run Python build script
python build.py --platform windows --arch x86_64 --build-type Release --clean

if errorlevel 1 (
    echo.
    echo Build failed!
    pause
    exit /b 1
) else (
    echo.
    echo Build successful!
)