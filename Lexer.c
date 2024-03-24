#include "Lexer.h"

#include <ctype.h>
#include <stdbool.h>

struct Lexer LexerNew(char *data, uint64_t dataCount)
{
    return (struct Lexer){
        .data = data,
        .dataCount = dataCount,
        .current = {0},
        .position = 0,
    };
}

char LexerChar(const struct Lexer *lexer)
{
    if (lexer->position >= lexer->dataCount)
    {
        return '\0';
    }

    return lexer->data[lexer->position];
}

struct Token LexerPeek(const struct Lexer *lexer)
{
    return lexer->current;
}

struct Token LexerNext(struct Lexer *lexer)
{
    struct Token token = lexer->current;
    lexer->current = LexerRead(lexer);

    return token;
}

struct Token LexerRead(struct Lexer *lexer)
{
    while (isspace(LexerChar(lexer)))
    {
        lexer->position += 1;
    }

    if (isalpha(LexerChar(lexer)))
    {
        // This is an identifier.
        // TODO: Identifiers should not contain ., :, etc. I'm just doing this right now as a quick hack. Those should be blocks, ie: (. a b c d) == a.b.c.d

        uint64_t start = lexer->position;

        while (isalnum(LexerChar(lexer)) || LexerChar(lexer) == '_' || LexerChar(lexer) == '.' || LexerChar(lexer) == ':')
        {
            lexer->position += 1;
        }

        uint64_t end = lexer->position;

        return (struct Token){
            .start = start,
            .end = end,
        };
    }

    if (LexerChar(lexer) == '"')
    {
        // This is a string.

        uint64_t start = lexer->position;
        lexer->position += 1;

        while (LexerChar(lexer) != '"')
        {
            lexer->position += 1;
        }

        lexer->position += 1;
        uint64_t end = lexer->position;

        return (struct Token){
            .start = start,
            .end = end,
        };
    }

    if (isdigit(LexerChar(lexer)))
    {
        // This is a number.

        uint64_t start = lexer->position;
        bool hasDecimal = false;

        while (isdigit(LexerChar(lexer)) || (!hasDecimal && LexerChar(lexer) == '.'))
        {
            if (LexerChar(lexer) == '.')
            {
                hasDecimal = true;
            }

            lexer->position += 1;
        }

        uint64_t end = lexer->position;

        return (struct Token){
            .start = start,
            .end = end,
        };
    }

    // TODO: Check if this is a two character operator.

    struct Token token = (struct Token){
        .start = lexer->position,
        .end = lexer->position + 1,
    };
    lexer->position += 1;

    return token;
}