@echo off
echo Building Hexa Language Interpreter (C Edition)

if not exist "build" mkdir build

gcc -Wall -Wextra -std=c99 -I./include -o build\hexai.exe src\main.c src\lexer.c src\parser.c src\value.c src\environment.c src\evaluator.c

if %errorlevel% neq 0 (
    echo Build failed!
    exit /b %errorlevel%
)

echo Build successful!
echo Executable is at build\hexai.exe 