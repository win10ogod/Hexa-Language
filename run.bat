@echo off
echo Running Hexa Program: %1

if not exist "build\hexai.exe" (
    echo Building Hexa interpreter first...
    call build.bat
    if %errorlevel% neq 0 (
        echo Build failed!
        exit /b %errorlevel%
    )
)

echo Running program...
build\hexai.exe %1

if %errorlevel% neq 0 (
    echo Program execution failed!
    exit /b %errorlevel%
)

echo Program executed successfully. 