@echo off
cd /d "%~dp0"
title MAGL - Uninstaller

echo.
echo.
echo        MAGL v2.1 - Uninstaller
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

echo        Searching for running MAGL processes...
tasklist /fi "imagename eq MAGL.exe" | find /i "MAGL.exe" >nul
if %errorLevel% == 0 (
    echo        Stopping MAGL process...
    taskkill /f /im MAGL.exe >nul 2>&1
    echo * [OK] MAGL process stopped
    echo.
    timeout /t 2 /nobreak >nul
) else (
    echo * [INFO] No running MAGL processes found
    echo.
)

echo        Removing startup shortcut...
set "shortcut_path=%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup\MAGL.lnk"

if exist "%shortcut_path%" (
    del "%shortcut_path%" >nul 2>&1
    echo * [OK] Startup shortcut removed
    echo.
) else (
    echo * [INFO] Startup shortcut was not found [already removed]
    echo.
)

echo        Removing settings file...
if exist "magl_settings.cfg" (
    del "magl_settings.cfg" >nul 2>&1
    echo * [OK] Settings file removed
    echo.
) else (
    echo * [INFO] Settings file was not found [already removed]
    echo.
)

echo.
echo        UNINSTALLATION COMPLETE!
echo.
echo * [OK] MAGL has been completely uninstalled!
echo.
echo        Note: MAGL.exe file is kept for future use
echo        Delete the entire folder manually if needed
echo.
pause