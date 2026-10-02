@echo off
title CRIMSON builder
echo ============================================
echo   CRIMSON - one-click build
echo ============================================
echo.

where cmake >nul 2>nul
if errorlevel 1 (
    echo [ERROR] CMake was not found.
    echo Install Visual Studio 2022 with the "Desktop development with C++" workload,
    echo then run this file again.
    echo.
    pause
    exit /b 1
)

echo [1/2] Configuring...
cmake --preset msvc-x64-release
if errorlevel 1 (
    echo.
    echo [ERROR] Configure failed. Read the messages above.
    pause
    exit /b 1
)

echo.
echo [2/2] Building...
cmake --build --preset msvc-x64-release
if errorlevel 1 (
    echo.
    echo [ERROR] Build failed. Read the messages above.
    pause
    exit /b 1
)

echo.
echo ============================================
echo   DONE!
echo   Your files are in the  out\bin  folder:
echo     crimson_injector.exe   ^(run this^)
echo     crimson_client.dll     ^(keep next to it^)
echo ============================================
echo.
start "" explorer "%~dp0out\bin"
pause
