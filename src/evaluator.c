#include "../include/hexa.h"
#include <stdarg.h>

// Forward declarations
static Value evaluateList(Value list, Environment* env);

// Helper for error reporting
static void runtimeErrorVA(const char* format, va_list args) {
    fprintf(stderr, "Runtime Error: ");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
}

void runtimeError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    runtimeErrorVA(format, args);
    va_end(args);
}

Value evaluate(Value expr, Environment* env) {
    switch (expr.type) {
        case VAL_NUMBER:
        case VAL_BOOLEAN:
        case VAL_STRING:
        case VAL_NIL:
        case VAL_FUNCTION:
        case VAL_NATIVE:
            return expr;
        case VAL_SYMBOL:
            return getVariable(env, expr.as.symbol);
        case VAL_LIST:
            return evaluateList(expr, env);
        default:
            runtimeError("Cannot evaluate unknown value type %d.", expr.type);
            return NIL_VAL;
    }
}

// Native function implementations
static Value nativePrint(int argCount, Value* args) {
    for (int i = 0; i < argCount; i++) {
        printValue(args[i]);
        printf(" ");
    }
    printf("\n");
    return NIL_VAL;
}

static Value nativeAdd(int argCount, Value* args) {
    if (argCount != 2) {
        runtimeError("Expected 2 arguments but got %d.", argCount);
        return NIL_VAL;
    }
    
    if (args[0].type == VAL_NUMBER && args[1].type == VAL_NUMBER) {
        return makeNumber(args[0].as.number + args[1].as.number);
    }
    
    runtimeError("Arguments must be numbers.");
    return NIL_VAL;
}

static Value nativeSubtract(int argCount, Value* args) {
    if (argCount != 2) {
        runtimeError("Expected 2 arguments but got %d.", argCount);
        return NIL_VAL;
    }
    
    if (args[0].type == VAL_NUMBER && args[1].type == VAL_NUMBER) {
        return makeNumber(args[0].as.number - args[1].as.number);
    }
    
    runtimeError("Arguments must be numbers.");
    return NIL_VAL;
}

static Value nativeMultiply(int argCount, Value* args) {
    if (argCount != 2) {
        runtimeError("Expected 2 arguments but got %d.", argCount);
        return NIL_VAL;
    }
    
    if (args[0].type == VAL_NUMBER && args[1].type == VAL_NUMBER) {
        return makeNumber(args[0].as.number * args[1].as.number);
    }
    
    runtimeError("Arguments must be numbers.");
    return NIL_VAL;
}

static Value nativeDivide(int argCount, Value* args) {
    if (argCount != 2) {
        runtimeError("Expected 2 arguments but got %d.", argCount);
        return NIL_VAL;
    }
    
    if (args[0].type == VAL_NUMBER && args[1].type == VAL_NUMBER) {
        if (args[1].as.number == 0) {
            runtimeError("Division by zero.");
            return NIL_VAL;
        }
        return makeNumber(args[0].as.number / args[1].as.number);
    }
    
    runtimeError("Arguments must be numbers.");
    return NIL_VAL;
}

static Value nativeEqual(int argCount, Value* args) {
    if (argCount != 2) {
        runtimeError("Expected 2 arguments but got %d.", argCount);
        return NIL_VAL;
    }
    
    if (args[0].type != args[1].type) {
        return makeBoolean(false);
    }
    
    switch (args[0].type) {
        case VAL_NIL:
            return makeBoolean(true);
        case VAL_BOOLEAN:
            return makeBoolean(args[0].as.boolean == args[1].as.boolean);
        case VAL_NUMBER:
            return makeBoolean(args[0].as.number == args[1].as.number);
        case VAL_STRING:
            return makeBoolean(strcmp(args[0].as.string, args[1].as.string) == 0);
        case VAL_SYMBOL:
            return makeBoolean(strcmp(args[0].as.symbol, args[1].as.symbol) == 0);
        default:
            return makeBoolean(false);
    }
}

static Value nativeLessThan(int argCount, Value* args) {
    if (argCount != 2) {
        runtimeError("Expected 2 arguments but got %d.", argCount);
        return NIL_VAL;
    }
    
    if (args[0].type == VAL_NUMBER && args[1].type == VAL_NUMBER) {
        return makeBoolean(args[0].as.number < args[1].as.number);
    }
    
    runtimeError("Arguments must be numbers.");
    return NIL_VAL;
}

static Value nativeGreaterThan(int argCount, Value* args) {
    if (argCount != 2) {
        runtimeError("Expected 2 arguments but got %d.", argCount);
        return NIL_VAL;
    }
    
    if (args[0].type == VAL_NUMBER && args[1].type == VAL_NUMBER) {
        return makeBoolean(args[0].as.number > args[1].as.number);
    }
    
    runtimeError("Arguments must be numbers.");
    return NIL_VAL;
}

static Value defineFn(int argCount, Value* args, Environment* env) {
    // (void)env; // Silence unused parameter warning - REMOVED, env is now used
    
    if (argCount < 2) {
        runtimeError("Expected at least 2 arguments but got %d.", argCount);
        return NIL_VAL;
    }
    
    // First argument should be the parameter list
    if (args[0].type != VAL_LIST) {
        runtimeError("Expected parameter list.");
        return NIL_VAL;
    }
    
    int arity = args[0].as.list.count;
    Value function_val = makeFunction(arity); // Renamed to avoid conflict with Function struct
    
    // Capture the current environment for closure
    function_val.as.function.captured_env = env;
    if (env != NULL) { // The function value now holds a reference
        env->ref_count++;
    }

    // Copy parameter names
    for (int i = 0; i < arity; i++) {
        if (args[0].as.list.items[i].type != VAL_SYMBOL) {
            runtimeError("Expected parameter name.");
            // TODO: Proper memory management for partially created function_val if error occurs
            return NIL_VAL;
        }
        appendToList(&function_val.as.function.params, copyValue(args[0].as.list.items[i]));
    }
    
    // Copy function body
    for (int i = 1; i < argCount; i++) {
        appendToList(&function_val.as.function.body, copyValue(args[i]));
    }
    
    return function_val;
}

static Value defineVar(int argCount, Value* args, Environment* env) {
    if (argCount != 2) {
        runtimeError("Expected 2 arguments but got %d.", argCount);
        return NIL_VAL;
    }
    
    // First argument should be the variable name
    if (args[0].type != VAL_SYMBOL) {
        runtimeError("Expected variable name.");
        return NIL_VAL;
    }
    
    // Second argument is the value
    Value value = evaluate(args[1], env);
    defineVariable(env, args[0].as.symbol, copyValue(value));
    
    return value;
}

static Value ifCondition(int argCount, Value* args, Environment* env) {
    if (argCount != 3) {
        runtimeError("Expected 3 arguments but got %d.", argCount);
        return NIL_VAL;
    }
    
    Value condition = evaluate(args[0], env);
    
    // Handle type conversion for the condition
    bool conditionResult = false;
    switch (condition.type) {
        case VAL_BOOLEAN:
            conditionResult = condition.as.boolean;
            break;
        case VAL_NUMBER:
            // Treat non-zero as true, zero as false
            conditionResult = condition.as.number != 0;
            break;
        case VAL_NIL:
            conditionResult = false;
            break;
        default:
            // Everything else (strings, symbols, lists, functions) is treated as true
            conditionResult = true;
            break;
    }
    
    if (conditionResult) {
        return evaluate(args[1], env);
    } else {
        return evaluate(args[2], env);
    }
}

static Value evaluateList(Value list, Environment* env) {
    if (list.as.list.count == 0) {
        return list;
    }
    
    // Evaluate the first element
    Value first = list.as.list.items[0];
    
    // Check for special forms
    if (first.type == VAL_SYMBOL) {
        // Define function
        if (strcmp(first.as.symbol, "fn") == 0) {
            return defineFn(list.as.list.count - 1, &list.as.list.items[1], env);
        }
        
        // Define variable
        if (strcmp(first.as.symbol, "def") == 0) {
            return defineVar(list.as.list.count - 1, &list.as.list.items[1], env);
        }
        
        // If condition
        if (strcmp(first.as.symbol, "if") == 0) {
            return ifCondition(list.as.list.count - 1, &list.as.list.items[1], env);
        }
    }
    
    // Function application
    Value evaluated = evaluate(first, env);
    
    // Prepare storage for evaluated arguments
    Value* args = malloc(sizeof(Value) * (list.as.list.count - 1));
    if (args == NULL) {
        runtimeError("Failed to allocate memory for function arguments.");
        return NIL_VAL;
    }
    
    // Evaluate arguments
    for (int i = 1; i < list.as.list.count; i++) {
        args[i - 1] = evaluate(list.as.list.items[i], env);
    }
    
    Value result = NIL_VAL;
    
    if (evaluated.type == VAL_FUNCTION) {
        Function function = evaluated.as.function;
        
        if (function.arity != list.as.list.count - 1) {
            runtimeError("Expected %d arguments but got %d.", function.arity, list.as.list.count - 1);
            free(args);
            return NIL_VAL;
        }
        
        // Create a new environment for the function execution, using the captured environment
        Environment* functionEnv = createEnclosedEnvironment(function.captured_env);
        
        // Bind arguments to parameters
        for (int i = 0; i < function.arity; i++) {
            if (function.params.items[i].type == VAL_SYMBOL) {
                defineVariable(functionEnv, function.params.items[i].as.symbol, copyValue(args[i]));
            } else {
                runtimeError("Invalid parameter name in function.");
                releaseEnvironmentReference(functionEnv);
                free(args);
                return NIL_VAL;
            }
        }
        
        // Evaluate the body in sequence, return the last result
        result = NIL_VAL;
        for (int i = 0; i < function.body.count; i++) {
            // Free previous result if not the last expression
            if (i > 0) {
                freeValue(result);
            }
            result = evaluate(function.body.items[i], functionEnv);
        }
        
        // Create a copy of the result before freeing the environment
        Value resultCopy = copyValue(result);
        
        // Release the function's execution environment
        releaseEnvironmentReference(functionEnv);
        
        // Return the copied result
        result = resultCopy;
    } else if (evaluated.type == VAL_NATIVE) {
        result = evaluated.as.native.function(list.as.list.count - 1, args);
    } else {
        runtimeError("Cannot call non-function. Got type %d.", evaluated.type);
    }
    
    // Free the argument array
    free(args);
    
    return result;
}

// Initialize the global environment with native functions
void initGlobalEnvironment(Environment* env) {
    defineVariable(env, "print", makeNative(nativePrint, "print"));
    defineVariable(env, "+", makeNative(nativeAdd, "+"));
    defineVariable(env, "-", makeNative(nativeSubtract, "-"));
    defineVariable(env, "*", makeNative(nativeMultiply, "*"));
    defineVariable(env, "/", makeNative(nativeDivide, "/"));
    defineVariable(env, "=", makeNative(nativeEqual, "="));
    defineVariable(env, "<", makeNative(nativeLessThan, "<"));
    defineVariable(env, ">", makeNative(nativeGreaterThan, ">"));
}