# Hexa Language Reference

Hexa is a square-bracket functional homoiconic language with a Lisp-like core but a C-like syntax.

## Syntax Basics

Hexa uses square brackets `[]` to denote lists and function calls. The first element of a list determines what happens with the rest of the elements.

### Data Types

Hexa supports the following primitive data types:

- **Numbers**: `123`, `45.6`
- **Strings**: `"Hello, world!"`
- **Booleans**: `true`, `false`
- **Nil**: `nil` (represents absence of a value)
- **Symbols**: `foo`, `+`, `bar-baz`, etc.
- **Lists**: `[1 2 3]`, `[foo bar]`, etc.
- **Functions**: `[fn [x y] [+ x y]]`

### Comments

Comments start with a semicolon (`;`) and continue to the end of the line:

```
; This is a comment
[print "Hello"] ; This is also a comment
```

## Core Functions

### Arithmetic Operations

- `[+ a b]` - Addition
- `[- a b]` - Subtraction
- `[* a b]` - Multiplication
- `[/ a b]` - Division

### Comparison Operations

- `[= a b]` - Equal to
- `[< a b]` - Less than
- `[> a b]` - Greater than

### Variables

Variables are defined using the `def` special form:

```
[def variable-name value]
```

Example:

```
[def x 10]
[def greeting "Hello, World!"]
```

### Functions

Functions are defined using the `fn` special form:

```
[fn [param1 param2 ...] body]
```

Example:

```
[fn [x y] [+ x y]]
```

To define a named function, combine `def` and `fn`:

```
[def add [fn [x y] [+ x y]]]
```

### Conditionals

Conditionals use the `if` special form:

```
[if condition then-expr else-expr]
```

Example:

```
[if [> x 10]
  [print "x is greater than 10"]
  [print "x is less than or equal to 10"]]
```

## Homoiconicity and Macros

Hexa is homoiconic, which means code is represented as data. This allows for powerful metaprogramming through macros.

In Hexa, macros are just regular functions that manipulate code (represented as lists) before it's evaluated. You define macros using standard function definitions.

Example macro:

```
[def when [fn [condition body]
  [if condition body nil]]]
```

Usage:

```
[when [> x 10] [print "x is greater than 10"]]
```

## Example Programs

### Hello World

```
[print "Hello, World!"]
```

### Factorial Function

```
[def factorial [fn [n]
  [if [< n 2]
    1
    [* n [factorial [- n 1]]]
  ]
]]

[print [factorial 5]] ; Prints 120
```

### Fibonacci Sequence

```
[def fibonacci [fn [n]
  [if [< n 2]
    n
    [+ [fibonacci [- n 1]] [fibonacci [- n 2]]]
  ]
]]

[print [fibonacci 10]] ; Prints 55
``` 