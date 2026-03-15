# Configure + Build (cmake -B build, then cmake --build build)
# Run in project root: .\configure_and_build.ps1

$ErrorActionPreference = "Stop"
$projDir = $PSScriptRoot
Set-Location $projDir

Write-Host "=== Step 1: Configure (cmake -B build) ===" -ForegroundColor Cyan
cmake -B build
if ($LASTEXITCODE -ne 0) {
    Write-Host "Configure failed. If OpenCV not found, set OpenCV_DIR or: cmake -DOpenCV_DIR=path -B build" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "=== Step 2: Build (cmake --build build) ===" -ForegroundColor Cyan
cmake --build build
if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed." -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "Done." -ForegroundColor Green
$exeRelease = Join-Path $projDir "build\Release\HandwritingNumberRecognition.exe"
$exeRoot = Join-Path $projDir "build\HandwritingNumberRecognition.exe"
if (Test-Path $exeRelease) {
    Write-Host "Exe: $exeRelease" -ForegroundColor Green
} elseif (Test-Path $exeRoot) {
    Write-Host "Exe: $exeRoot" -ForegroundColor Green
}
Write-Host "Copy mnist-fc folder next to the exe before running." -ForegroundColor Yellow
