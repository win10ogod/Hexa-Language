#include "../include/hexa.h"
#include <string.h> // Ensure string.h is included for strlen/strcpy
#include <stdio.h> // For NULL

// Helper function to actually free environment data when ref_count is 0
static void trulyFreeEnvironment(Environment* env) {
    if (env == NULL) return;
    for (int i = 0; i < env->count; i++) {
        free(env->entries[i].key); // Free the key string
        freeValue(env->entries[i].value); // Recursively free the value
    }
    free(env->entries);
    // Recursively release the enclosing environment if it exists
    if (env->enclosing != NULL) {
        releaseEnvironmentReference(env->enclosing);
    }
    free(env);
}

void releaseEnvironmentReference(Environment* env) {
    if (env == NULL) return;
    env->ref_count--;
    if (env->ref_count <= 0) {
        trulyFreeEnvironment(env);
    }
}

Environment* createEnvironment() {
    Environment* env = malloc(sizeof(Environment));
    if (env == NULL) {
        runtimeError("Memory allocation failed for new environment.");
        return NULL; // Should handle this error more robustly
    }
    env->count = 0;
    env->capacity = 0;
    env->entries = NULL;
    env->enclosing = NULL;
    env->ref_count = 1; // Initial reference count
    return env;
}

Environment* createEnclosedEnvironment(Environment* enclosing) {
    Environment* env = createEnvironment();
    if (env == NULL) return NULL; // createEnvironment might fail

    env->enclosing = enclosing;
    if (enclosing != NULL) {
        enclosing->ref_count++; // Increment ref_count of the enclosed (outer) environment
    }
    return env;
}

// void freeEnvironment(Environment* env) { // OLD FUNCTION - replaced by releaseEnvironmentReference
//     for (int i = 0; i < env->count; i++) {
//         free(env->entries[i].key);
//         freeValue(env->entries[i].value);
//     }
//     free(env->entries);
//     // if (env->enclosing) freeEnvironment(env->enclosing); // This was problematic
//     free(env);
// }

static void ensureCapacity(Environment* env) {
    if (env->capacity < env->count + 1) {
        int oldCapacity = env->capacity;
        env->capacity = oldCapacity < 8 ? 8 : oldCapacity * 2;
        Entry* new_entries = realloc(env->entries, sizeof(Entry) * env->capacity);
        if (new_entries == NULL && env->capacity > 0) {
            runtimeError("Memory reallocation failed for environment entries.");
            // Potentially exit or handle error more robustly
            return;
        }
        env->entries = new_entries;
    }
}

void defineVariable(Environment* env, const char* name, Value value) {
    if (env == NULL) {
        runtimeError("Cannot define variable in NULL environment.");
        return;
    }
    // Check if variable already exists in the current environment
    for (int i = 0; i < env->count; i++) {
        if (strcmp(env->entries[i].key, name) == 0) {
            freeValue(env->entries[i].value); // Free the old value
            env->entries[i].value = value;    // Assign the new value (ownership transferred)
            return;
        }
    }
    
    // Add new entry
    ensureCapacity(env);
    if (env->entries == NULL && env->capacity > 0) return; // ensureCapacity might have failed and returned

    char* key_copy = malloc(strlen(name) + 1);
    if (key_copy == NULL) {
        runtimeError("Memory allocation failed for variable name key.");
        freeValue(value); // Clean up the value that won't be stored
        return;
    }
    strcpy(key_copy, name);
    env->entries[env->count].key = key_copy;
    env->entries[env->count].value = value; // Ownership of value transferred to env
    env->count++;
}

Value getVariable(Environment* env, const char* name) {
    if (env == NULL) {
        runtimeError("Cannot get variable from NULL environment.");
        return NIL_VAL;
    }
    // Search in current environment
    for (int i = 0; i < env->count; i++) {
        if (strcmp(env->entries[i].key, name) == 0) {
            return copyValue(env->entries[i].value); // Return a copy
        }
    }
    
    // Search in enclosing environment
    if (env->enclosing != NULL) {
        return getVariable(env->enclosing, name);
    }
    
    // Variable not found
    runtimeError("Undefined variable '%s'.", name);
    return NIL_VAL;
}

bool assignVariable(Environment* env, const char* name, Value value) {
    if (env == NULL) {
        runtimeError("Cannot assign variable in NULL environment.");
        return false;
    }
    // Search in current environment
    for (int i = 0; i < env->count; i++) {
        if (strcmp(env->entries[i].key, name) == 0) {
            freeValue(env->entries[i].value); // Free the old value
            env->entries[i].value = value;    // Assign the new value (ownership transferred)
            return true;
        }
    }
    
    // Search in enclosing environment
    if (env->enclosing != NULL) {
        return assignVariable(env->enclosing, name, value);
    }
    
    // Variable not found
    runtimeError("Cannot assign to undefined variable '%s'.", name);
    freeValue(value); // Clean up the value that won't be stored
    return false;
}



// void initGlobalEnvironment(Environment* env) { // MOVED TO EVALUATOR.C
//     if (env == NULL) return;
//     // Native functions are defined here. Values returned by makeNative are owned by the environment.
//     defineVariable(env, "print", makeNative(nativePrint, "print"));
//     defineVariable(env, "+", makeNative(nativeAdd, "+"));
//     defineVariable(env, "-", makeNative(nativeSubtract, "-"));
//     defineVariable(env, "*", makeNative(nativeMultiply, "*"));
//     defineVariable(env, "/", makeNative(nativeDivide, "/"));
//     defineVariable(env, "=", makeNative(nativeEqual, "="));
//     defineVariable(env, "<", makeNative(nativeLessThan, "<"));
//     defineVariable(env, ">", makeNative(nativeGreaterThan, ">"));
//     // Add other built-ins as needed
// }

