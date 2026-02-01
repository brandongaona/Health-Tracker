@echo off
cd /d "%~dp0"
set GCC=C:\msys64\mingw64\bin\g++.exe
echo Compiling server.cpp and healthtracker.cpp...
"%GCC%" -o server.exe server.cpp healthtracker.cpp -std=c++17 -pthread -D_WIN32_WINNT=0x0A00 -lws2_32 > build_log.txt 2>&1
set BUILD_EXIT=%ERRORLEVEL%
type build_log.txt
if %BUILD_EXIT% NEQ 0 (
    echo Build FAILED with exit code %BUILD_EXIT%
    exit /b %BUILD_EXIT%
)
echo Build succeeded. Output: server.exe
exit /b 0
