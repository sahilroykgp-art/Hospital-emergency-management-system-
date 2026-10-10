@echo off
echo ====================================================
echo  Building Hospital Emergency Management System
echo ====================================================

if not exist bin mkdir bin

gcc -Wall -Wextra -std=c11 -Iinclude src\*.c -o hospital_er.exe

if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] Build complete! Executable: hospital_er.exe
) else (
    echo [ERROR] Compilation failed!
    exit /b %ERRORLEVEL%
)
