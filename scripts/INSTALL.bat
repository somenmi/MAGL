@echo off
cd /d "%~dp0"
title MAGL - Installer

echo.
echo.
echo        MAGL v2.1 - Installer
echo.
echo    Dev: Yalkee
echo    GitHub: https://github.com/somenmi/MAGL
echo.
echo.

echo        Checking administrator privileges...
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo * [ERR] Run as Administrator!
    echo.
    pause
    exit /b 1
)

echo * [OK] Administrator privileges confirmed
echo.

echo        Checking MAGL.exe...
if not exist "MAGL.exe" (
    echo * [ERR] MAGL.exe not found!
    echo.
    pause
    exit /b 1
)

echo * [OK] MAGL.exe found
echo.

echo        Creating startup shortcut...
set "shortcut_path=%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup\MAGL.lnk"

powershell -Command "$s=(New-Object -COM WScript.Shell).CreateShortcut('%shortcut_path%'); $s.TargetPath='%CD%\MAGL.exe'; $s.Arguments='-tray'; $s.WorkingDirectory='%CD%'; $s.WindowStyle=7; $s.IconLocation='%CD%\MAGL.exe,0'; $s.Save()"

echo * [OK] Startup shortcut created
echo * [OK] Parameter: -tray [start in system tray only]
echo.

echo        Starting MAGL...
start "" "MAGL.exe" -tray

echo.
echo        INSTALLATION COMPLETE!
echo.
echo * [OK] MAGL has been installed successfully!
echo.
echo        Features:
echo          - Auto-start with Windows directly to system tray
echo          - No window on startup - only tray icon  
echo          - Protects microphone volume automatically
echo.
echo        To configure: Double-click system tray icon
echo        To uninstall: Run UNINSTALL.bat
echo.
echo        The program is now running in system tray...
echo.
pause