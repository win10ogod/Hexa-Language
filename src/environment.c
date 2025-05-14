#include "../include/hexa.h"

Environment* createEnvironment() {
    Environment* env = malloc(sizeof(Environment));
    env->count = 0;
    env->capacity = 0;
    env->entries = NULL;
    env->enclosing = NULL;
    return env;
}

Environment* createEnclosedEnvironment(Environment* enclosing) {
    Environment* env = createEnvironment();
    env->enclosing = enclosing;
    return env;
}

void freeEnvironment(Environment* env) {
    for (int i = 0; i < env->count; i++) {
        free(env->entries[i].key);
        freeValue(env->entries[i].value);
    }
    
    free(env->entries);
    free(env);
}

static void ensureCapacity(Environment* env) {
    if (env->capacity < env->count + 1) {
        int oldCapacity = env->capacity;
        env->capacity = oldCapacity < 8 ? 8 : oldCapacity * 2;
        env->entries = realloc(env->entries, sizeof(Entry) * env->capacity);
    }
}

void defineVariable(Environment* env, const char* name, Value value) {
    // Check if variable already exists
    for (int i = 0; i < env->count; i++) {
        if (strcmp(env->entries[i].key, name) == 0) {
            freeValue(env->entries[i].value);
            env->entries[i].value = value;
            return;
        }
    }
    
    // Add new entry
    ensureCapacity(env);
    env->entries[env->count].key = strdup(name);
    env->entries[env->count].value = value;
    env->count++;
}

Value getVariable(Environment* env, const char* name) {
    // Search in current environment
    for (int i = 0; i < env->count; i++) {
        if (strcmp(env->entries[i].key, name) == 0) {
            return env->entries[i].value;
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
    // Search in current environment
    for (int i = 0; i < env->count; i++) {
        if (strcmp(env->entries[i].key, name) == 0) {
            freeValue(env->entries[i].value);
            env->entries[i].value = value;
            return true;
        }
    }
    
    // Search in enclosing environment
    if (env->enclosing != NULL) {
        return assignVariable(env->enclosing, name, value);
    }
    
    // Variable not found
    runtimeError("Undefined variable '%s'.", name);
    return false;
} 