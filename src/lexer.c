#include "../include/hexa.h"

typedef struct {
    const char* start;
    const char* current;
    int line;
} Lexer;

static Lexer lexer;

void initLexer(const char* source) {
    lexer.start = source;
    lexer.current = source;
    lexer.line = 1;
}

static bool isAtEnd() {
    return *lexer.current == '\0';
}

static char advance() {
    return *lexer.current++;
}

static char peek() {
    return *lexer.current;
}

static char peekNext() {
    if (isAtEnd()) return '\0';
    return lexer.current[1];
}

// Note: This function is currently not used but kept for future extensions
#ifdef UNUSED
static bool match(char expected) {
    if (isAtEnd()) return false;
    if (*lexer.current != expected) return false;
    lexer.current++;
    return true;
}
#endif

static Token makeToken(TokenType type) {
    Token token;
    token.type = type;
    token.lexeme = (char*)lexer.start;
    token.length = (int)(lexer.current - lexer.start);
    token.line = lexer.line;
    return token;
}

static Token errorToken(const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.lexeme = (char*)message;
    token.length = (int)strlen(message);
    token.line = lexer.line;
    return token;
}

static void skipWhitespace() {
    for (;;) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                lexer.line++;
                advance();
                break;
            case ';':
                // Comment goes until the end of the line
                while (peek() != '\n' && !isAtEnd()) advance();
                break;
            default:
                return;
        }
    }
}

static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

static bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_' || c == '-' || c == '+' || c == '*' || 
           c == '/' || c == '=' || c == '>' || c == '<' || 
           c == '!' || c == '?' || c == '&' || c == '|' || 
           c == '%' || c == '^' || c == '~';
}

static Token number() {
    while (isDigit(peek())) advance();

    // Look for a fractional part
    if (peek() == '.' && isDigit(peekNext())) {
        // Consume the '.'
        advance();

        while (isDigit(peek())) advance();
    }

    return makeToken(TOKEN_NUMBER);
}

static Token identifier() {
    while (isAlpha(peek()) || isDigit(peek())) advance();

    // Check for keywords
    int length = (int)(lexer.current - lexer.start);
    
    if (length == 4 && memcmp(lexer.start, "true", 4) == 0) {
        return makeToken(TOKEN_BOOLEAN);
    }
    if (length == 5 && memcmp(lexer.start, "false", 5) == 0) {
        return makeToken(TOKEN_BOOLEAN);
    }
    if (length == 3 && memcmp(lexer.start, "nil", 3) == 0) {
        return makeToken(TOKEN_NIL);
    }

    return makeToken(TOKEN_IDENTIFIER);
}

static Token string() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') lexer.line++;
        advance();
    }

    if (isAtEnd()) return errorToken("Unterminated string.");

    // The closing quote
    advance();
    return makeToken(TOKEN_STRING);
}

Token scanToken() {
    skipWhitespace();
    lexer.start = lexer.current;

    if (isAtEnd()) return makeToken(TOKEN_EOF);

    char c = advance();

    if (isDigit(c)) return number();
    if (isAlpha(c)) return identifier();

    switch (c) {
        case '[': return makeToken(TOKEN_LBRACKET);
        case ']': return makeToken(TOKEN_RBRACKET);
        case '"': return string();
    }

    return errorToken("Unexpected character.");
} 