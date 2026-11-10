@echo off
if not defined VS170COMNTOOLS (
    echo Please, run this script from "Developer Command Prompt for VS 2022".
    exit /b 1
)
cd /d "%~dp0"
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release