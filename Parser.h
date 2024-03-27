#pragma once

#include "Lexer.h"
#include "Block.h"

typedef struct Parser
{
    Lexer lexer;
    Font *font;
} Parser;

Parser ParserNew(Lexer lexer, Font *font);

void ParserMatch(Parser *parser, char *string);
bool ParserHas(Parser *parser, char *string);
void ParserList(Parser *parser, int32_t parentI,
    int32_t (*ParserFunction)(Parser *parser, int32_t parentI, int32_t childI), int32_t startI, char *end,
    char *separator);

int32_t ParserParseDo(Parser *parser, int32_t parentI, int32_t childI);
int32_t ParserParseCase(Parser *parser, int32_t parentI, int32_t childI);
int32_t ParserParseIfCases(Parser *parser, int32_t parentI, int32_t childI);
int32_t ParserParseElseCase(Parser *parser, int32_t parentI, int32_t childI);
int32_t ParserParseIf(Parser *parser, int32_t parentI, int32_t childI);
int32_t ParserParseStatementList(Parser *parser, int32_t parentI, int32_t childI);
int32_t ParserParseFunctionHeader(Parser *parser, int32_t parentI, int32_t childI);
int32_t ParserParseFunction(Parser *parser, int32_t parentI, int32_t childI);
int32_t ParserParseLambdaFunctionHeader(Parser *parser, int32_t parentI, int32_t childI);
int32_t ParserParseLambdaFunction(Parser *parser, int32_t parentI, int32_t childI);
int32_t ParserParseAddition(Parser *parser, int32_t parentI, int32_t childI);
int32_t ParserParseUnarySuffix(Parser *parser, int32_t parentI, int32_t childI);
int32_t ParserParsePrimary(Parser *parser, int32_t parentI, int32_t childI);

int32_t ParserParseExpression(Parser *parser, int32_t parentI, int32_t childI);
int32_t ParserParseStatement(Parser *parser, int32_t parentI, int32_t childI);
int32_t ParserParseIdentifier(Parser *parser, int32_t parentI, int32_t childI);