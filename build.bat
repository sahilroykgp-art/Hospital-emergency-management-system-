@echo off
setlocal
echo ====================================================
echo  Building Hospital Emergency Management System
echo ====================================================

if not exist bin mkdir bin

echo Compiling main application (hospital_er.exe)...
gcc -Wall -Wextra -std=c11 -Iinclude src/admin.c src/bed_manager.c src/doctor.c src/file_io.c src/main.c src/patient.c src/priority_queue.c src/receptionist.c src/triage.c -o hospital_er.exe

if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] Build complete! Executable: hospital_er.exe
) else (
    echo [ERROR] Compilation failed!
    exit /b %ERRORLEVEL%
)

echo.
echo Compiling automated test suite (test_runner.exe)...
gcc -Wall -Wextra -std=c11 -Iinclude src/patient.c src/triage.c src/priority_queue.c src/bed_manager.c src/file_io.c tests/test_priority_queue.c -o test_runner.exe

if %ERRORLEVEL% EQU 0 (
    copy /y test_runner.exe bin\test_runner.exe >nul 2>&1
    echo [SUCCESS] Test suite built: test_runner.exe
) else (
    echo [ERROR] Test suite compilation failed!
    exit /b %ERRORLEVEL%
)
