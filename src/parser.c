#include "../include/hexa.h"

typedef struct {
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
} Parser;

static Parser parser;

// Forward declarations
static Value expression();
static Value parseList();
static void advance();

static void errorAt(Token* token, const char* message) {
    if (parser.panicMode) return;
    parser.panicMode = true;
    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {
        // Nothing
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->lexeme);
    }

    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;
}

static void errorAtCurrent(const char* message) {
    errorAt(&parser.current, message);
}

// Implementation of the error function (no longer static)
void error(const char* message) {
    errorAt(&parser.previous, message);
}

static void advance() {
    parser.previous = parser.current;

    for (;;) {
        parser.current = scanToken();
        if (parser.current.type != TOKEN_ERROR) break;

        errorAtCurrent(parser.current.lexeme);
    }
}

// Initialize the parser
void initParser() {
    parser.hadError = false;
    parser.panicMode = false;
    advance(); // Read the first token
}

// Get the current token
Token getCurrentToken() {
    return parser.current;
}

static void consume(TokenType type, const char* message) {
    if (parser.current.type == type) {
        advance();
        return;
    }

    errorAtCurrent(message);
}

static bool check(TokenType type) {
    return parser.current.type == type;
}

static Value number() {
    double value = strtod(parser.previous.lexeme, NULL);
    return makeNumber(value);
}

static Value string() {
    // Extract the string without quotes
    char* str = malloc(parser.previous.length - 1);
    if (str == NULL) {
        error("Out of memory");
        return NIL_VAL;
    }
    
    // Copy string content without quotes
    memcpy(str, parser.previous.lexeme + 1, parser.previous.length - 2);
    str[parser.previous.length - 2] = '\0';
    
    Value val = makeString(str);
    free(str);
    return val;
}

static Value boolean() {
    if (strncmp(parser.previous.lexeme, "true", 4) == 0) {
        return makeBoolean(true);
    } else {
        return makeBoolean(false);
    }
}

static Value nil() {
    return NIL_VAL;
}

static Value identifier() {
    // Extract the symbol
    char* symbol = malloc(parser.previous.length + 1);
    if (symbol == NULL) {
        error("Out of memory");
        return NIL_VAL;
    }
    
    // Copy symbol
    memcpy(symbol, parser.previous.lexeme, parser.previous.length);
    symbol[parser.previous.length] = '\0';
    
    Value val = makeSymbol(symbol);
    free(symbol);
    return val;
}

static Value primary() {
    switch (parser.current.type) {
        case TOKEN_NUMBER: {
            advance();
            return number();
        }
        case TOKEN_STRING: {
            advance();
            return string();
        }
        case TOKEN_BOOLEAN: {
            advance();
            return boolean();
        }
        case TOKEN_NIL: {
            advance();
            return nil();
        }
        case TOKEN_IDENTIFIER: {
            advance();
            return identifier();
        }
        case TOKEN_LBRACKET: {
            return parseList();
        }
        default: {
            error("Expected expression.");
            return NIL_VAL;
        }
    }
}

static Value parseList() {
    Value list = makeList();
    
    // Consume the opening '['
    consume(TOKEN_LBRACKET, "Expected '['.");
    
    // Parse expressions until we hit a closing ']'
    while (!check(TOKEN_RBRACKET) && !check(TOKEN_EOF)) {
        Value expr = expression();
        appendToList(&list.as.list, expr);
    }
    
    consume(TOKEN_RBRACKET, "Expected ']' after list.");
    
    return list;
}

static Value expression() {
    return primary();
}

// Parse a single expression (for use with multiple expressions)
Value parseExpression() {
    Value expr = expression();
    return expr;
}

// Original parse function for backward compatibility
Value parse(const char* source) {
    initLexer(source);
    
    parser.hadError = false;
    parser.panicMode = false;
    
    advance();
    Value result = expression();
    
    consume(TOKEN_EOF, "Expected end of expression.");
    
    return result;
} 