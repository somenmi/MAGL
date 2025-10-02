@echo off
chcp 65001
title MAGL - Установка

echo ============================================
echo.
echo      MAGL (MicAutoGainLock) - Установка
echo.
echo ============================================
echo.
echo  Разработчик: Yalkee
echo  GitHub: https://github.com/somenmi/MAGL
echo.
echo ============================================
echo.

cd /d "%~dp0"

echo  Текущая папка: %CD%
echo.

if not exist "MAGL.exe" (
    echo ❌ Файл MAGL.exe не найден!
    echo Сначала запусти COMPILE.bat
    echo.
    pause
    exit /b
)

echo  ✅ Файл MAGL.exe найден

net session >nul 2>&1
if %errorLevel% NEQ 0 (
    echo ❌ ЗАПУСТИ ОТ ИМЕНИ АДМИНИСТРАТОРА!
    echo.
    pause
    exit /b
)

echo Создаем ярлык в автозагрузке...
set SCRIPT="%TEMP%\%RANDOM%-%RANDOM%-%RANDOM%-%RANDOM%.vbs"

echo Set oWS = WScript.CreateObject("WScript.Shell") > %SCRIPT%
echo sLinkFile = "%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup\MAGL.lnk" >> %SCRIPT%
echo Set oLink = oWS.CreateShortcut(sLinkFile) >> %SCRIPT%
echo oLink.TargetPath = "%~dp0MAGL.exe" >> %SCRIPT%
echo oLink.WorkingDirectory = "%~dp0" >> %SCRIPT%
echo oLink.Description = "MicAutoGainLock - Skajem net krokozyambram" >> %SCRIPT%
echo oLink.WindowStyle = 7 >> %SCRIPT%
echo oLink.IconLocation = "%~dp0MAGL.exe, 0" >> %SCRIPT%
echo oLink.Save >> %SCRIPT%

cscript /nologo %SCRIPT%
del %SCRIPT%

echo  Запускаем программу...
start "" "MAGL.exe"

echo.
echo ✅ УСТАНОВКА ЗАВЕРШЕНА!
echo.
echo - MAGL теперь защищает ваш микрофон!
echo - Программа будет запускаться автоматически при входе в систему
echo - Для удаления запусти UNINSTALL.bat
echo.
echo ПЕРЕЗАГРУЗИТЕ КОМПЬЮТЕР ДЛЯ ПРОВЕРКИ!
echo.
pause