@echo off
cd /d "%~dp0"
echo Starting Health Tracker Server...
echo Working directory: %CD%
echo.

REM Verify required files exist
if not exist server.cpp (echo ERROR: server.cpp not found. & pause & exit /b 1)
if not exist healthtracker.cpp (echo ERROR: healthtracker.cpp not found. & pause & exit /b 1)
if not exist planner.h (echo ERROR: planner.h not found. & pause & exit /b 1)
if not exist httplib.h (echo ERROR: httplib.h not found. & pause & exit /b 1)
if not exist json.hpp (echo ERROR: json.hpp not found. & pause & exit /b 1)
echo All source files and headers found.
echo.

REM Use current dir for temp files so g++ does not use missing S:\ from a previous session
set TMP=%CD%
set TEMP=%CD%

"C:\msys64\mingw64\bin\g++.exe" -o server.exe server.cpp healthtracker.cpp -std=c++17 -pthread -D_WIN32_WINNT=0x0A00 -lws2_32
echo.
echo Exit code: %ERRORLEVEL%
if %ERRORLEVEL% NEQ 0 (
    echo Build failed! See errors above.
    echo.
    echo If you see no errors above, the path may have spaces ^(e.g. "Brandon Gaona"^).
    echo Open "MSYS2 MinGW 64-bit" and run the build from there. See BUILD.md for the command.
    pause
    exit /b 1
)
echo Build successful! Starting server...
server.exe
