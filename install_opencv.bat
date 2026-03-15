@echo off
cd /d "%~dp0"
echo Running OpenCV installer script (download + extract to C:\opencv)...
echo.
powershell -ExecutionPolicy Bypass -File "%~dp0install_opencv.ps1"
echo.
pause
