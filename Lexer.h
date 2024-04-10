#pragma once

#include <inttypes.h>
#include <stdbool.h>

typedef struct Token
{
    int32_t start;
    int32_t end;
} Token;

typedef struct Lexer
{
    char *data;
    int32_t dataCount;
    Token current;
    int32_t position;
} Lexer;

Lexer LexerNew(char *data, int32_t dataCount);
char LexerChar(const Lexer *lexer);
char LexerPeekChar(const Lexer *lexer);
Token LexerPeek(const Lexer *lexer);
Token LexerNext(Lexer *lexer);
Token LexerRead(Lexer *lexer);
bool LexerTokenEquals(Lexer *lexer, Token token, char *string, bool isPrefix);