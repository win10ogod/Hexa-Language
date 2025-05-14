#include "../include/hexa.h"
#include <assert.h>

// Forward declaration
void initGlobalEnvironment(Environment* env);
void printValue(Value value);

static void testLexer() {
    printf("Testing lexer...\n");
    
    initLexer("[print 123 \"hello\"]");
    
    Token token;
    
    token = scanToken();
    assert(token.type == TOKEN_LBRACKET);
    
    token = scanToken();
    assert(token.type == TOKEN_IDENTIFIER);
    assert(strncmp(token.lexeme, "print", token.length) == 0);
    
    token = scanToken();
    assert(token.type == TOKEN_NUMBER);
    assert(strncmp(token.lexeme, "123", token.length) == 0);
    
    token = scanToken();
    assert(token.type == TOKEN_STRING);
    
    token = scanToken();
    assert(token.type == TOKEN_RBRACKET);
    
    token = scanToken();
    assert(token.type == TOKEN_EOF);
    
    printf("Lexer tests passed!\n");
}

static void testParser() {
    printf("Testing parser...\n");
    
    Value expr = parse("[print 123]");
    
    assert(expr.type == VAL_LIST);
    assert(expr.as.list.count == 2);
    assert(expr.as.list.items[0].type == VAL_SYMBOL);
    assert(strcmp(expr.as.list.items[0].as.symbol, "print") == 0);
    assert(expr.as.list.items[1].type == VAL_NUMBER);
    assert(expr.as.list.items[1].as.number == 123);
    
    freeValue(expr);
    
    printf("Parser tests passed!\n");
}

static void testEvaluator() {
    printf("Testing evaluator...\n");
    
    Environment* env = createEnvironment();
    initGlobalEnvironment(env);
    
    // Test addition
    Value expr = parse("[+ 1 2]");
    Value result = evaluate(expr, env);
    
    assert(result.type == VAL_NUMBER);
    assert(result.as.number == 3);
    
    freeValue(expr);
    
    // Test function definition and application
    expr = parse("[def add [fn [a b] [+ a b]]]");
    evaluate(expr, env);
    freeValue(expr);
    
    expr = parse("[add 5 7]");
    result = evaluate(expr, env);
    
    assert(result.type == VAL_NUMBER);
    assert(result.as.number == 12);
    
    freeValue(expr);
    freeEnvironment(env);
    
    printf("Evaluator tests passed!\n");
}

int main() {
    testLexer();
    testParser();
    testEvaluator();
    
    printf("All tests passed!\n");
    return 0;
} 