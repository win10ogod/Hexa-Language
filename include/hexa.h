#ifndef HEXA_H
#define HEXA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Type definitions
typedef enum {
    TOKEN_EOF,
    TOKEN_LBRACKET,     // [
    TOKEN_RBRACKET,     // ]
    TOKEN_IDENTIFIER,   // symbols, names
    TOKEN_STRING,       // "text"
    TOKEN_NUMBER,       // 123, 45.6
    TOKEN_BOOLEAN,      // true, false
    TOKEN_NIL,          // nil
    TOKEN_COMMENT,      // ; comment
    TOKEN_ERROR         // For error tokens
} TokenType;

typedef struct {
    TokenType type;
    char* lexeme;
    int length;
    int line;
} Token;

// Value types
typedef enum {
    VAL_NIL,
    VAL_BOOLEAN,
    VAL_NUMBER,
    VAL_STRING,
    VAL_SYMBOL,
    VAL_LIST,
    VAL_FUNCTION,
    VAL_NATIVE
} ValueType;

typedef struct Value Value;
typedef struct List List;
typedef Value (*NativeFn)(int argCount, Value* args);

struct List {
    int count;
    int capacity;
    Value* items;
};

typedef struct {
    NativeFn function;
    const char* name;
} NativeFunction;

typedef struct {
    int arity;
    List body;
    List params;
    struct Environment* captured_env; // Added for closure support
} Function;

struct Value {
    ValueType type;
    union {
        bool boolean;
        double number;
        char* string;
        char* symbol;
        List list;
        Function function;
        NativeFunction native;
    } as;
};

// Define nil value
#define NIL_VAL ((Value){VAL_NIL, {.number = 0}})

// Utility functions
Value makeNumber(double num);
Value makeBoolean(bool value);
Value makeString(const char* string);
Value makeSymbol(const char* symbol);
Value makeList();
Value makeFunction(int arity);
Value makeNative(NativeFn function, const char* name);

// List functions
void initList(List* list);
void freeList(List* list);
void appendToList(List* list, Value value);

// Value functions
void printValue(Value value);
void freeValue(Value value);
Value copyValue(Value value);
bool valuesEqual(Value a, Value b);

// Environment
typedef struct {
    char* key;
    Value value;
} Entry;

typedef struct Environment {
    int count;
    int capacity;
    Entry* entries;
    struct Environment* enclosing;
    int ref_count; // Added for reference counting
} Environment;

// Function prototypes for lexer
void initLexer(const char* source);
Token scanToken();

// Function prototypes for parser
void initParser();
Token getCurrentToken();
Value parseExpression();
Value parse(const char* source);

// Function prototypes for evaluator
Value evaluate(Value expr, Environment* env);
Environment* createEnvironment();
Environment* createEnclosedEnvironment(Environment* enclosing);
void defineVariable(Environment* env, const char* name, Value value);
Value getVariable(Environment* env, const char* name);
bool assignVariable(Environment* env, const char* name, Value value);
// void freeEnvironment(Environment* env); // Old
void releaseEnvironmentReference(Environment* env); // New name
void initGlobalEnvironment(Environment* env);

// Error handling
void error(const char* message);
void runtimeError(const char* format, ...);

#endif // HEXA_H 