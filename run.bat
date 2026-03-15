@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"

REM OpenCV bin: use first argument, or default (your Downloads path)
if not "%~1"=="" (
    for %%A in ("%~1") do set "OCV_BUILD=%%~fA"
) else (
    set "OCV_BUILD=C:\Users\36255\Downloads\opencv\build"
)
set "OCV_BIN=!OCV_BUILD!\x64\vc16\bin"
if exist "!OCV_BIN!" set "PATH=!OCV_BIN!;!PATH!"

REM Copy mnist-fc to build\Debug if needed
if exist "mnist-fc\meta.json" if not exist "build\Debug\mnist-fc\meta.json" (
    echo Copying mnist-fc to build\Debug...
    xcopy /E /I /Y "mnist-fc" "build\Debug\mnist-fc" >nul
)

if exist "build\Debug\HandwritingNumberRecognition.exe" (
    echo Starting HandwritingNumberRecognition...
    "build\Debug\HandwritingNumberRecognition.exe" %2 %3 %4
) else if exist "build\Release\HandwritingNumberRecognition.exe" (
    if exist "mnist-fc\meta.json" if not exist "build\Release\mnist-fc\meta.json" (
        xcopy /E /I /Y "mnist-fc" "build\Release\mnist-fc" >nul
    )
    echo Starting HandwritingNumberRecognition...
    "build\Release\HandwritingNumberRecognition.exe" %2 %3 %4
) else (
    echo HandwritingNumberRecognition.exe not found. Build first: .\configure_and_build.bat "OpenCV\build\path"
    pause
    exit /b 1
)
pause
