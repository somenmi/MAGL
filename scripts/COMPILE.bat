@echo off
cd /d "%~dp0"
title MAGL - Compiler

echo.
echo.
echo        MAGL v2.1 - Compiler
echo.
echo    Dev: Yalkee
echo    GitHub: https://github.com/somenmi/MAGL
echo.
echo.

echo        Compiling MAGL v2.1 with icons...
echo.

windres MAGL.rc -o MAGL_res.o
g++ -o MAGL.exe MAGL.cpp MAGL_res.o -static -lole32 -lwinmm -lcomctl32 -mwindows -municode
del MAGL_res.o 2>nul

if exist "MAGL.exe" (
    echo [OK] SUCCESS! MAGL.exe with icons created.
    echo - [OK] Window icon
    echo - [OK] Tray icon
    echo - [OK] Professional GUI
    echo.
    pause
) else (
    echo [X] FAILED! Check files:
    echo - MAGL.cpp
    echo - MAGL.ico  
    echo - MAGL.rc
    echo.
    pause
)