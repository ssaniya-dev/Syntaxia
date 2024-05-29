#include <stdio.h>
#include <stdlib.h>

typedef enum {
    SEMI, 
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
} TypeSymbol;

typedef enum {
    STOP,
} TypeKeyword;

typedef enum {
    INTEGER,
} TypeLiteral;

typedef struct {
    TypeKeyword type;
} TokenKeyword;

typedef struct {
    TypeLiteral type;
} TokenLiteral;

typedef struct {
    TypeSymbol type;
} TokenSymbol;

int main() {
    printf("Hello, World!\n");
    return 0;
}