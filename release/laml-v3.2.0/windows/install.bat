@echo off
echo 🚀 LAML Windows Installer
echo ========================
echo This will install LAML on your Windows system.
echo.
echo Administrator privileges required.
echo.
pause
powershell -ExecutionPolicy Bypass -File "%~dp0install.ps1"
pause
