@echo off
chcp 65001
title MAGL - Компиляция

echo ============================================
echo.
echo     MAGL (MicAutoGainLock) - Компиляция
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

if not exist "MAGL.cpp" (
    echo    ❌ Файл MAGL.cpp не найден!
    echo.
    pause
    exit /b
)

echo ✅ Файл MAGL.cpp найден
echo.

set COMPILER=
if exist "C:\mingw64\bin\g++.exe" set COMPILER=C:\mingw64\bin\g++
if exist "C:\msys64\mingw64\bin\g++.exe" set COMPILER=C:\msys64\mingw64\bin\g++
if exist "g++.exe" set COMPILER=g++

if "%COMPILER%"=="" (
    echo ❌ Компилятор C++ не найден!
    echo.
    echo Установи MinGW-w64 или добавь g++ в PATH
    echo.
    pause
    exit /b
)

echo ✅ Компилятор найден: %COMPILER%
echo.

if exist "MAGL.ico" (
    echo ✅ Иконка MAGL.ico найдена
    echo.
) else (
    echo ℹ️  Иконка MAGL.ico не найдена - будет использована стандартная
)

if exist "MAGL.rc" (
    echo ✅ Файл ресурсов MAGL.rc найден
    echo.
    echo ============================================
    echo Компилируем с иконкой и информацией о версии...
    
    windres MAGL.rc -o MAGL_res.o
    
    %COMPILER% -o MAGL.exe MAGL.cpp MAGL_res.o -static -lole32 -lwinmm
    
    del MAGL_res.o >nul 2>&1
) else (
    echo ℹ️  Файл ресурсов MAGL.rc не найден - компилируем без иконки
    %COMPILER% -o MAGL.exe MAGL.cpp -static -lole32 -lwinmm
)

if exist "MAGL.exe" (
    echo ============================================
    echo.
    echo      ✅ MAGL УСПЕШНО СКОМПИЛИРОВАН!
    echo.
    echo    Информация о версии добавлена:
    echo    - CompanyName: Yalkee
    echo    - FileDescription: MAGL - MicAutoGainLock  
    echo    - Version: 2.0.0.0
    echo    - Copyright: 2025 Yalkee
    echo.
    echo  - - - - - - - - - - - - - - - - - - - - - -
    echo.
    echo  p.s.: Уменьшает ложные срабатывания WinDef
    echo.
    echo ============================================
    echo.
    echo * Особенности программы:
    echo  - Реагирует ТОЛЬКО при изменении громкости Mic
    echo  - Восстанавливает через N секунду
    echo  - Минимальная нагрузка на систему
    echo.
    echo ============================================
    echo.
    echo Теперь запусти: INSTALL.bat
    echo.
) else (
    echo ❌ Ошибка компиляции! 👉 p.s.: но если все ✅ горят, значит не ошибка C:
    echo.
)

pause