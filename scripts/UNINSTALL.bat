@echo off
chcp 65001
title MAGL - Удаление

echo ============================================
echo.
echo      MAGL (MicAutoGainLock) - Удаление
echo.
echo ============================================
echo.
echo  Разработчик: Yalkee
echo  GitHub: https://github.com/somenmi/MAGL
echo.
echo ============================================
echo.

cd /d "%~dp0"

echo  Останавливаем MAGL...
taskkill /f /im MAGL.exe >nul 2>&1

echo  Удаляем ярлык из автозагрузки...
del "%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup\MAGL.lnk" /f /q >nul 2>&1

echo  Удаляем задачу из Планировщика...
schtasks /delete /tn "MAGL" /f >nul 2>&1

echo.
echo  ✅ MAGL полностью удален!
pause