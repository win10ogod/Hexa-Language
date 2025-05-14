#include "../include/hexa.h"

// Forward declaration of the global environment initializer
void initGlobalEnvironment(Environment* env);
void appendToList(List* list, Value value);
void printValue(Value value);

static void repl(Environment* env) {
    char line[1024];
    
    for (;;) {
        printf("> ");
        
        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }
        
        Value expr = parse(line);
        Value result = evaluate(expr, env);
        
        printf("=> ");
        printValue(result);
        printf("\n");
        
        freeValue(expr);
    }
}

static char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }
    
    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);
    
    char* buffer = malloc(fileSize + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }
    
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(74);
    }
    
    buffer[bytesRead] = '\0';
    
    fclose(file);
    return buffer;
}

// Debug function to print tokens
static void debugTokens(const char* source) {
    initLexer(source);
    
    int line = -1;
    for (;;) {
        Token token = scanToken();
        if (token.line != line) {
            printf("%4d ", token.line);
            line = token.line;
        } else {
            printf("   | ");
        }
        
        printf("%-12s '%.*s'\n", 
               token.type == TOKEN_EOF ? "EOF" :
               token.type == TOKEN_LBRACKET ? "LBRACKET" :
               token.type == TOKEN_RBRACKET ? "RBRACKET" :
               token.type == TOKEN_IDENTIFIER ? "IDENTIFIER" :
               token.type == TOKEN_STRING ? "STRING" :
               token.type == TOKEN_NUMBER ? "NUMBER" :
               token.type == TOKEN_BOOLEAN ? "BOOLEAN" :
               token.type == TOKEN_NIL ? "NIL" :
               token.type == TOKEN_COMMENT ? "COMMENT" :
               token.type == TOKEN_ERROR ? "ERROR" : "UNKNOWN",
               token.length, token.lexeme);
               
        if (token.type == TOKEN_EOF) break;
    }
}

// Create a function to parse and evaluate multiple expressions
static void parseAndEvaluateMultiple(const char* source, Environment* env) {
    // Initialize the lexer with the source
    initLexer(source);
    
    // Create a parser context
    initParser();
    
    // Parse and evaluate expressions until we reach the end of the file
    while (getCurrentToken().type != TOKEN_EOF) {
        Value expr = parseExpression();
        Value result = evaluate(expr, env);
        
        // Only print non-nil results
        if (result.type != VAL_NIL) {
            printValue(result);
            printf("\n");
        }
        
        freeValue(expr);
        // Don't free result here as it might be part of the environment
    }
}

static void runFile(const char* path, Environment* env) {
    char* source = readFile(path);
    
    // Debug tokens only when requested
    // debugTokens(source);
    
    // Parse and evaluate multiple expressions
    parseAndEvaluateMultiple(source, env);
    
    free(source);
}

int main(int argc, char* argv[]) {
    // Create global environment
    Environment* globalEnv = createEnvironment();
    initGlobalEnvironment(globalEnv);
    
    if (argc == 1) {
        // No arguments, run REPL
        printf("Hexa Language Interpreter (C Edition)\n");
        printf("Press Ctrl+C to exit\n");
        repl(globalEnv);
    } else if (argc == 2) {
        // One argument, run file
        runFile(argv[1], globalEnv);
    } else if (argc == 3 && strcmp(argv[1], "--debug") == 0) {
        // Debug mode
        char* source = readFile(argv[2]);
        debugTokens(source);
        free(source);
    } else {
        fprintf(stderr, "Usage: hexai [path]\n");
        exit(64);
    }
    
    freeEnvironment(globalEnv);
    return 0;
} 