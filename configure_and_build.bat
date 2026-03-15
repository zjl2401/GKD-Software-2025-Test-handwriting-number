@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"
echo === Step 1: Configure (cmake -B build) ===
if not "%~1"=="" (
    for %%A in ("%~1") do set "OCV_RAW=%%~fA"
    set "OCV_CMAKE=!OCV_RAW:\=/!"
    set "OpenCV_DIR=!OCV_RAW!"
    echo Using OpenCV at: !OCV_RAW!
    if exist build\CMakeCache.txt del /q build\CMakeCache.txt
)
if not "%~1"=="" (
    cmake -DOpenCV_DIR="!OCV_CMAKE!" -B build
) else (
    cmake -B build
)
if errorlevel 1 (
    echo.
    echo Configure failed. OpenCV not found.
    echo Use path that contains OpenCVConfig.cmake:  .\configure_and_build.bat "C:\path\to\opencv\build"
    pause
    exit /b 1
)
echo.
echo === Step 2: Build (cmake --build build) ===
cmake --build build
if errorlevel 1 (
    echo Build failed.
    pause
    exit /b 1
)
echo.
echo Done. Copy mnist-fc folder next to HandwritingNumberRecognition.exe before running.
if exist "build\Release\HandwritingNumberRecognition.exe" echo Exe: build\Release\HandwritingNumberRecognition.exe
if exist "build\HandwritingNumberRecognition.exe" echo Exe: build\HandwritingNumberRecognition.exe
pause
