#pragma once

#include <inttypes.h>

struct Token
{
    uint64_t start;
    uint64_t end;
};

struct Lexer
{
    char *data;
    uint64_t dataCount;
    struct Token current;
    uint64_t position;
};

struct Lexer LexerNew(char *data, uint64_t dataCount);
char LexerChar(const struct Lexer *lexer);
struct Token LexerPeek(const struct Lexer *lexer);
struct Token LexerNext(struct Lexer *lexer);
struct Token LexerRead(struct Lexer *lexer);