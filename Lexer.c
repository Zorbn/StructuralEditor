#include "Lexer.h"

#include <ctype.h>

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

char LexerChar(const Lexer *lexer)
{
    if (lexer->position >= lexer->dataCount)
    {
        return '\0';
    }

    return lexer->data[lexer->position];
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
        // TODO: Identifiers should not contain ., :, etc. I'm just doing this right now as a quick hack. Those should be blocks, ie: (. a b c d) == a.b.c.d

        int32_t start = lexer->position;

        while (isalnum(LexerChar(lexer)) || LexerChar(lexer) == '_' || LexerChar(lexer) == '.' || LexerChar(lexer) == ':')
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

        while (LexerChar(lexer) != '"')
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

    // TODO: Check if this is a two character operator.

    Token token = (Token){
        .start = lexer->position,
        .end = lexer->position + 1,
    };
    lexer->position += 1;

    return token;
}

bool LexerTokenEquals(Lexer* lexer, Token token, char *string)
{
    for (int32_t i = token.start; i < token.end; i++)
    {
        int32_t stringI = i - token.start;

        if (string[stringI] != lexer->data[i])
        {
            return false;
        }
    }

    return string[token.end - token.start] == '\0';
}