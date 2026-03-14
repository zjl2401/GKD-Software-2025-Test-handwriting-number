# Build script - Compile project using g++
# Usage: .\build.ps1

$projDir = $PSScriptRoot
$opencvDir = "C:\Users\yaping\Downloads\opencv-4.12.0"
$buildDir = Join-Path $projDir "build"

# Create build directory
if (-not (Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir | Out-Null
}

# 确保 nlohmann/json 已拉取（通过 CMake FetchContent）
$jsonInclude = Join-Path $buildDir "_deps\json-src\single_include"
if (-not (Test-Path (Join-Path $jsonInclude "nlohmann\json.hpp"))) {
    Write-Host "Running CMake to fetch nlohmann/json..." -ForegroundColor Yellow
    cmake -B build 2>$null
}

Write-Host "Compiling..." -ForegroundColor Green

# Compile source file
Write-Host "Compiling main.cpp..." -ForegroundColor Yellow
g++ -std=c++17 `
    -I"$projDir\include" `
    -I"$jsonInclude" `
    -I"$opencvDir\build" `
    -I"$opencvDir\include" `
    -I"$opencvDir\modules\core\include" `
    -I"$opencvDir\modules\imgproc\include" `
    -I"$opencvDir\modules\highgui\include" `
    -I"$opencvDir\modules\videoio\include" `
    -I"$opencvDir\modules\imgcodecs\include" `
    -c "$projDir\src\main.cpp" `
    -o "$buildDir\main.o"

if ($LASTEXITCODE -ne 0) {
    Write-Host "Compilation failed!" -ForegroundColor Red
    exit 1
}

# Link
Write-Host "Linking executable..." -ForegroundColor Yellow
g++ "$buildDir\main.o" `
    -L"$opencvDir\build\lib" `
    -lopencv_highgui4120 `
    -lopencv_videoio4120 `
    -lopencv_imgcodecs4120 `
    -lopencv_imgproc4120 `
    -lopencv_core4120 `
    -o "$buildDir\HandwritingNumberRecognition.exe"

if ($LASTEXITCODE -ne 0) {
    Write-Host "Linking failed! Error code: $LASTEXITCODE" -ForegroundColor Red
    Write-Host "Please check OpenCV library path and library names" -ForegroundColor Yellow
    exit 1
} else {
    if (Test-Path "$buildDir\HandwritingNumberRecognition.exe") {
        Write-Host "Linking successful!" -ForegroundColor Green
    } else {
        Write-Host "Warning: Link command succeeded but exe file not generated!" -ForegroundColor Yellow
    }
}

# Copy OpenCV DLL files to build directory
Write-Host "Copying OpenCV DLL files..." -ForegroundColor Yellow
$opencvBinDir = Join-Path $opencvDir "build\bin"
$requiredDlls = @(
    "libopencv_core4120.dll",
    "libopencv_imgproc4120.dll",
    "libopencv_imgcodecs4120.dll",
    "libopencv_videoio4120.dll",
    "libopencv_highgui4120.dll"
)

foreach ($dll in $requiredDlls) {
    $srcDll = Join-Path $opencvBinDir $dll
    $dstDll = Join-Path $buildDir $dll
    if (Test-Path $srcDll) {
        Copy-Item $srcDll $dstDll -Force
        Write-Host "  Copied: $dll" -ForegroundColor Gray
    }
}

Write-Host "Build successful!" -ForegroundColor Green
$exePath = Join-Path $buildDir "HandwritingNumberRecognition.exe"
Write-Host "Executable: $exePath" -ForegroundColor Green
