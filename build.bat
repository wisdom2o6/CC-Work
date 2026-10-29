@echo off
setlocal
echo ===================================================
echo     BIEN DICH DU AN TETRIS 3D (OPENGL + C++)
echo ===================================================
echo Dang bien dich ma nguon...

set GXX=w64devkit\bin\g++.exe

if not exist "%GXX%" (
    echo [LOI] Khong tim thay trinh bien dich tai %GXX%!
    pause
    exit /b 1
)

"%GXX%" -O2 -std=c++17 ^
    src\BlockLogic.cpp ^
    src\GameLogic.cpp ^
    src\Camera.cpp ^
    src\FontRenderer.cpp ^
    src\Renderer.cpp ^
    src\Audio.cpp ^
    src\Input.cpp ^
    src\main.cpp ^
    -o Tetris3D.exe ^
    -lopengl32 -lglu32 -lgdi32 -luser32 -lwinmm

if %ERRORLEVEL% equ 0 (
    echo.
    echo ===================================================
    echo [THANH CONG] Da tao file thuc thi: Tetris3D.exe
    echo Ban co the khoi dong game bang cach chay Tetris3D.exe!
    echo ===================================================
) else (
    echo.
    echo [THAT BAI] Co loi trong qua trinh bien dich!
)

pause
