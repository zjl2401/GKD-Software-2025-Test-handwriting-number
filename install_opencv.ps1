# Download and extract OpenCV for Windows (run from project root or any folder)
# Usage: powershell -ExecutionPolicy Bypass -File install_opencv.ps1
#    or: .\install_opencv.ps1   (if execution policy allows)

$OpenCVVersion = "4.10.0"
$InstallRoot = "C:\opencv"
$Url = "https://github.com/opencv/opencv/releases/download/$OpenCVVersion/opencv-$OpenCVVersion-windows.exe"
$ExePath = "$env:TEMP\opencv-$OpenCVVersion-windows.exe"

Write-Host "=== OpenCV $OpenCVVersion for Windows ===" -ForegroundColor Cyan
Write-Host "Download URL: $Url"
Write-Host "Install to:   $InstallRoot"
Write-Host ""

# Create target directory
if (-not (Test-Path $InstallRoot)) {
    New-Item -ItemType Directory -Path $InstallRoot -Force | Out-Null
    Write-Host "Created directory: $InstallRoot"
}

# Download
Write-Host "Downloading OpenCV (this may take a few minutes)..." -ForegroundColor Yellow
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
try {
    Invoke-WebRequest -Uri $Url -OutFile $ExePath -UseBasicParsing
} catch {
    Write-Host "Download failed: $_" -ForegroundColor Red
    Write-Host "You can manually download from: $Url" -ForegroundColor Yellow
    exit 1
}
Write-Host "Download done: $ExePath" -ForegroundColor Green

# Extract: the .exe is a self-extracting archive, often supports /S /D=path
Write-Host "Extracting to $InstallRoot ..." -ForegroundColor Yellow
$proc = Start-Process -FilePath $ExePath -ArgumentList "/S", "/D=$InstallRoot" -Wait -PassThru
if ($proc.ExitCode -ne 0) {
    Write-Host "Silent extract may have failed (exit code $($proc.ExitCode))." -ForegroundColor Yellow
    Write-Host "Opening installer for you - please choose install path: $InstallRoot" -ForegroundColor Yellow
    Start-Process -FilePath $ExePath -Wait
}

# Find OpenCVConfig.cmake
$cfg = Get-ChildItem -Path $InstallRoot -Recurse -Filter "OpenCVConfig.cmake" -ErrorAction SilentlyContinue | Select-Object -First 1
if ($cfg) {
    $dir = $cfg.DirectoryName
    Write-Host ""
    Write-Host "OpenCV extracted. OpenCVConfig.cmake found at:" -ForegroundColor Green
    Write-Host "  $dir"
    Write-Host ""
    Write-Host "Next step - build this project with:" -ForegroundColor Cyan
    Write-Host "  .\configure_and_build.bat `"$dir`""
} else {
    Write-Host ""
    Write-Host "Extraction finished. OpenCVConfig.cmake was not found under $InstallRoot" -ForegroundColor Yellow
    Write-Host "If you ran the installer manually, check that you selected: $InstallRoot"
    Write-Host "Then run: Get-ChildItem -Path C:\opencv -Recurse -Filter OpenCVConfig.cmake | % { $_.DirectoryName }"
    Write-Host "Use that directory path with: .\configure_and_build.bat `"<path>`""
}

Remove-Item $ExePath -Force -ErrorAction SilentlyContinue
Write-Host ""
Write-Host "Done." -ForegroundColor Green
