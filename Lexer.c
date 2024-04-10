#include "Lexer.h"

#include <ctype.h>
#include <string.h>

Lexer LexerNew(char *data, int32_t dataCount)
{
    Lexer lexer = (Lexer){
        .data = data,
        .dataCount = dataCount,
        .current = {0},
        .position = 0,
    };

    lexer.current = LexerRead(&lexer);

    return lexer;
}

static char LexerGetChar(const Lexer *lexer, int32_t i)
{
    if (i >= lexer->dataCount)
    {
        return '\0';
    }

    return lexer->data[i];
}

char LexerChar(const Lexer *lexer)
{
    return LexerGetChar(lexer, lexer->position);
}

char LexerPeekChar(const Lexer *lexer)
{
    return LexerGetChar(lexer, lexer->position + 1);
}

Token LexerPeek(const Lexer *lexer)
{
    return lexer->current;
}

Token LexerNext(Lexer *lexer)
{
    Token token = lexer->current;
    lexer->current = LexerRead(lexer);

    return token;
}

Token LexerRead(Lexer *lexer)
{
    while (isspace(LexerChar(lexer)))
    {
        lexer->position += 1;
    }

    if (isalpha(LexerChar(lexer)))
    {
        // This is an identifier.
        // TODO: Identifiers should not contain ., :, [, ], etc. I'm just doing this right now as a quick hack. Those should
        // be blocks, ie: (. a b c d) == a.b.c.d

        int32_t start = lexer->position;

        while (isalnum(LexerChar(lexer)) || LexerChar(lexer) == '_' || LexerChar(lexer) == '.' ||
               LexerChar(lexer) == ':' || LexerChar(lexer) == '[' || LexerChar(lexer) == ']')
        {
            lexer->position += 1;
        }

        int32_t end = lexer->position;

        return (Token){
            .start = start,
            .end = end,
        };
    }

    if (LexerChar(lexer) == '"')
    {
        // This is a string.

        int32_t start = lexer->position;
        lexer->position += 1;

        while (LexerChar(lexer) != '"' && LexerChar(lexer) != '\0')
        {
            lexer->position += 1;
        }

        lexer->position += 1;
        int32_t end = lexer->position;

        return (Token){
            .start = start,
            .end = end,
        };
    }

    if (isdigit(LexerChar(lexer)))
    {
        // This is a number.

        int32_t start = lexer->position;
        bool hasDecimal = false;

        while (isdigit(LexerChar(lexer)) || (!hasDecimal && LexerChar(lexer) == '.'))
        {
            if (LexerChar(lexer) == '.')
            {
                hasDecimal = true;
            }

            lexer->position += 1;
        }

        int32_t end = lexer->position;

        return (Token){
            .start = start,
            .end = end,
        };
    }

    if (LexerChar(lexer) == '.' && LexerPeekChar(lexer) == '.')
    {
        Token token = (Token){
            .start = lexer->position,
            .end = lexer->position + 2,
        };
        lexer->position += 2;

        return token;
    }

    if (LexerPeekChar(lexer) == '=')
    {
        switch (LexerChar(lexer))
        {
        case '<':
        case '>':
        case '=':
        case '~': {
            Token token = (Token){
                .start = lexer->position,
                .end = lexer->position + 2,
            };
            lexer->position += 2;

            return token;
        }
        }
    }

    if (LexerChar(lexer) == '-' && LexerPeekChar(lexer) == '-')
    {
        int32_t start = lexer->position;

        while (LexerChar(lexer) != '\r' && LexerChar(lexer) != '\n' && LexerChar(lexer) != '\0')
        {
            lexer->position += 1;
        }

        int32_t end = lexer->position;

        return (Token){
            .start = start,
            .end = end,
        };
    }

    Token token = (Token){
        .start = lexer->position,
        .end = lexer->position + 1,
    };
    lexer->position += 1;

    return token;
}

bool LexerTokenEquals(Lexer *lexer, Token token, char *string, bool isPrefix)
{
    for (int32_t i = token.start; i < token.end; i++)
    {
        int32_t stringI = i - token.start;

        if (string[stringI] == '\0')
        {
            return isPrefix;
        }

        if (string[stringI] != lexer->data[i])
        {
            return false;
        }
    }

    return isPrefix || string[token.end - token.start] == '\0';
}