#include "../include/hexa.h"
#include <string.h> // Ensure string.h is included for strlen/strcpy

Value makeNumber(double num) {
    Value value;
    value.type = VAL_NUMBER;
    value.as.number = num;
    return value;
}

Value makeBoolean(bool b) {
    Value value;
    value.type = VAL_BOOLEAN;
    value.as.boolean = b;
    return value;
}

Value makeString(const char* string) {
    Value value;
    value.type = VAL_STRING;
    char* new_string = malloc(strlen(string) + 1);
    if (new_string == NULL) {
        runtimeError("Memory allocation failed for string.");
        // Ideally, handle this more gracefully, perhaps by returning an error value
        // For now, it might lead to issues if not checked by caller, but fixes strdup issue
        value.as.string = NULL; // Or some other indicator of failure
        return value; // Or an error Value
    }
    strcpy(new_string, string);
    value.as.string = new_string;
    return value;
}

Value makeSymbol(const char* symbol) {
    Value value;
    value.type = VAL_SYMBOL;
    char* new_symbol = malloc(strlen(symbol) + 1);
    if (new_symbol == NULL) {
        runtimeError("Memory allocation failed for symbol.");
        value.as.symbol = NULL;
        return value;
    }
    strcpy(new_symbol, symbol);
    value.as.symbol = new_symbol;
    return value;
}

Value makeList() {
    Value value;
    value.type = VAL_LIST;
    value.as.list.count = 0;
    value.as.list.capacity = 0;
    value.as.list.items = NULL;
    return value;
}

Value makeFunction(int arity) {
    Value value;
    value.type = VAL_FUNCTION;
    value.as.function.arity = arity;
    value.as.function.body.count = 0;
    value.as.function.body.capacity = 0;
    value.as.function.body.items = NULL;
    value.as.function.params.count = 0;
    value.as.function.params.capacity = 0;
    value.as.function.params.items = NULL;
    value.as.function.captured_env = NULL; // Initialize captured_env
    return value;
}

Value makeNative(NativeFn function, const char* name) {
    Value value;
    value.type = VAL_NATIVE;
    value.as.native.function = function;
    value.as.native.name = name; // Assuming name is a literal or managed elsewhere
    return value;
}

void initList(List* list) {
    list->count = 0;
    list->capacity = 0;
    list->items = NULL;
}

void freeList(List* list) {
    free(list->items);
    initList(list);
}

void appendToList(List* list, Value value) {
    if (list->capacity < list->count + 1) {
        int oldCapacity = list->capacity;
        list->capacity = oldCapacity < 8 ? 8 : oldCapacity * 2;
        list->items = realloc(list->items, sizeof(Value) * list->capacity);
        if (list->items == NULL && list->capacity > 0) {
             runtimeError("Memory reallocation failed for list items.");
             // Potentially exit or handle error more robustly
             return;
        }
    }

    list->items[list->count] = value;
    list->count++;
}

void printValue(Value value) {
    switch (value.type) {
        case VAL_NIL:
            printf("nil");
            break;
        case VAL_BOOLEAN:
            printf("%s", value.as.boolean ? "true" : "false");
            break;
        case VAL_NUMBER:
            printf("%g", value.as.number);
            break;
        case VAL_STRING:
            if (value.as.string) printf("\"%s\"", value.as.string);
            else printf("\"(null string error)\"");
            break;
        case VAL_SYMBOL:
            if (value.as.symbol) printf("%s", value.as.symbol);
            else printf("(null symbol error)");
            break;
        case VAL_LIST: {
            printf("[");
            for (int i = 0; i < value.as.list.count; i++) {
                printValue(value.as.list.items[i]);
                if (i < value.as.list.count - 1) printf(" ");
            }
            printf("]");
            break;
        }
        case VAL_FUNCTION:
            printf("[fn ");
            printf("[");
            for (int i = 0; i < value.as.function.params.count; i++) {
                printValue(value.as.function.params.items[i]);
                if (i < value.as.function.params.count - 1) printf(" ");
            }
            printf("] ");
            // Print body
            for (int i = 0; i < value.as.function.body.count; i++) {
                printValue(value.as.function.body.items[i]);
                if (i < value.as.function.body.count - 1) printf(" ");
            }
            printf("]");
            break;
        case VAL_NATIVE:
            printf("[native-fn %s]", value.as.native.name);
            break;
    }
}

void freeValue(Value value) {
    switch (value.type) {
        case VAL_STRING:
            free(value.as.string);
            break;
        case VAL_SYMBOL:
            free(value.as.symbol);
            break;
        case VAL_LIST:
            for (int i = 0; i < value.as.list.count; i++) {
                freeValue(value.as.list.items[i]);
            }
            freeList(&value.as.list);
            break;
        case VAL_FUNCTION:
            for (int i = 0; i < value.as.function.params.count; i++) {
                freeValue(value.as.function.params.items[i]);
            }
            freeList(&value.as.function.params);
            
            for (int i = 0; i < value.as.function.body.count; i++) {
                freeValue(value.as.function.body.items[i]);
            }
            freeList(&value.as.function.body);
            if (value.as.function.captured_env != NULL) {
                releaseEnvironmentReference(value.as.function.captured_env);
            }
            break;
        default:
            break;
    }
}

Value copyValue(Value value) {
    switch (value.type) {
        case VAL_NIL:
            return NIL_VAL;
        case VAL_BOOLEAN:
            return makeBoolean(value.as.boolean);
        case VAL_NUMBER:
            return makeNumber(value.as.number);
        case VAL_STRING:
            return value.as.string ? makeString(value.as.string) : (Value){VAL_STRING, {.string = NULL}};
        case VAL_SYMBOL:
            return value.as.symbol ? makeSymbol(value.as.symbol) : (Value){VAL_SYMBOL, {.symbol = NULL}};
        case VAL_LIST: {
            Value copy = makeList();
            for (int i = 0; i < value.as.list.count; i++) {
                appendToList(&copy.as.list, copyValue(value.as.list.items[i]));
            }
            return copy;
        }
        case VAL_FUNCTION: {
            Value copy = makeFunction(value.as.function.arity);
            
            // Copy parameters
            for (int i = 0; i < value.as.function.params.count; i++) {
                appendToList(&copy.as.function.params, copyValue(value.as.function.params.items[i]));
            }
            
            // Copy body
            for (int i = 0; i < value.as.function.body.count; i++) {
                appendToList(&copy.as.function.body, copyValue(value.as.function.body.items[i]));
            }
            copy.as.function.captured_env = value.as.function.captured_env; // Shallow copy of env pointer
            if (copy.as.function.captured_env != NULL) {
                copy.as.function.captured_env->ref_count++; // Increment ref_count when function is copied
            }
            return copy;
        }
        case VAL_NATIVE:
            return makeNative(value.as.native.function, value.as.native.name);
    }
    
    // Should never reach here
    runtimeError("Unknown value type in copyValue: %d", value.type);
    return NIL_VAL;
}

bool valuesEqual(Value a, Value b) {
    if (a.type != b.type) return false;
    
    switch (a.type) {
        case VAL_NIL:
            return true;
        case VAL_BOOLEAN:
            return a.as.boolean == b.as.boolean;
        case VAL_NUMBER:
            return a.as.number == b.as.number;
        case VAL_STRING:
            if (a.as.string == NULL && b.as.string == NULL) return true;
            if (a.as.string == NULL || b.as.string == NULL) return false;
            return strcmp(a.as.string, b.as.string) == 0;
        case VAL_SYMBOL:
            if (a.as.symbol == NULL && b.as.symbol == NULL) return true;
            if (a.as.symbol == NULL || b.as.symbol == NULL) return false;
            return strcmp(a.as.symbol, b.as.symbol) == 0;
        case VAL_LIST:
            if (a.as.list.count != b.as.list.count) return false;
            for (int i = 0; i < a.as.list.count; i++) {
                if (!valuesEqual(a.as.list.items[i], b.as.list.items[i])) return false;
            }
            return true;
        case VAL_FUNCTION:
        case VAL_NATIVE:
            // Functions and natives are only equal if they are the same object (pointer comparison for body/params/env)
            // For simplicity, we'll treat distinct function objects as non-equal, even if identical code/env.
            // A more robust equality would be complex.
            return &a.as.function == &b.as.function; // Basic pointer compare for struct
    }
    
    // Should never reach here
    runtimeError("Unknown value type in valuesEqual: %d", a.type);
    return false;
}

