#include "../include/hexa.h"

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
    value.as.string = strdup(string);
    return value;
}

Value makeSymbol(const char* symbol) {
    Value value;
    value.type = VAL_SYMBOL;
    value.as.symbol = strdup(symbol);
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
    return value;
}

Value makeNative(NativeFn function, const char* name) {
    Value value;
    value.type = VAL_NATIVE;
    value.as.native.function = function;
    value.as.native.name = name;
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
            printf("\"%s\"", value.as.string);
            break;
        case VAL_SYMBOL:
            printf("%s", value.as.symbol);
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
            return makeString(value.as.string);
        case VAL_SYMBOL:
            return makeSymbol(value.as.symbol);
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
            
            return copy;
        }
        case VAL_NATIVE:
            return makeNative(value.as.native.function, value.as.native.name);
    }
    
    // Should never reach here
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
            return strcmp(a.as.string, b.as.string) == 0;
        case VAL_SYMBOL:
            return strcmp(a.as.symbol, b.as.symbol) == 0;
        case VAL_LIST:
            if (a.as.list.count != b.as.list.count) return false;
            for (int i = 0; i < a.as.list.count; i++) {
                if (!valuesEqual(a.as.list.items[i], b.as.list.items[i])) return false;
            }
            return true;
        case VAL_FUNCTION:
        case VAL_NATIVE:
            // Functions and natives are only equal if they are the same object
            return false;
    }
    
    // Should never reach here
    return false;
} 