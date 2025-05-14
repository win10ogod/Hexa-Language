@echo off
echo Building Hexa Language Test Suite

if not exist "build" mkdir build

gcc -Wall -Wextra -std=c99 -I./include -o build\test.exe tests\test.c src\lexer.c src\parser.c src\value.c src\environment.c src\evaluator.c

if %errorlevel% neq 0 (
    echo Build failed!
    exit /b %errorlevel%
)

echo Running tests...
build\test.exe

if %errorlevel% neq 0 (
    echo Tests failed!
    exit /b %errorlevel%
)

echo All tests passed! 