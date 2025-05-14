# Hexa Language Implementation (C Edition)

A C-based interpreter for the Hexa language, targeting Windows platforms. Hexa is a square-bracket functional homoiconic language with a Lisp-like foundation but a more familiar C-like syntax.

## Project Structure

- `src/`: Source code for the interpreter
- `include/`: Header files
- `tests/`: Test cases for the interpreter
- `examples/`: Example Hexa programs
- `docs/`: Documentation

## Features

- **Square bracket syntax**: Uses `[` and `]` for delimiting expressions instead of parentheses
- **Homoiconicity**: Code as data, enabling powerful macro capabilities
- **First-class functions**: Functions are values that can be passed around
- **Lexical scoping**: Variables are scoped according to their lexical context
- **Dynamic typing**: Types are determined at runtime
- **REPL**: Interactive development environment

## Building

### Prerequisites

- Windows OS
- GCC or MinGW compiler

### Build Instructions

Run the build script:

```
build.bat
```

This will compile the interpreter to `build/hexai.exe`.

### Running Tests

To run the test suite:

```
test.bat
```

## Running Hexa Programs

You can run Hexa programs using the run script:

```
run.bat examples/hello.hexa
```

Or by directly calling the interpreter:

```
build\hexai.exe examples/hello.hexa
```

## Using the REPL

To start the interactive REPL (Read-Eval-Print Loop):

```
build\hexai.exe
```

## Example Programs

Several example programs are included in the `examples/` directory:

- `hello.hexa`: Basic Hello World and function examples
- `macros.hexa`: Examples of macro usage

## Language Documentation

See the `docs/language_reference.md` file for a comprehensive guide to the Hexa language syntax and features.

## License

This project is released under the MIT License. 