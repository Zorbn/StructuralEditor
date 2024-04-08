#pragma once

#include "Lexer.h"
#include "Block.h"
#include "List.h"

typedef struct Parser
{
    Lexer lexer;
    Font *font;
    List_char textBuffer;
} Parser;

Parser ParserNew(Lexer lexer, Font *font);
void ParserDelete(Parser *parser);

void ParserMatch(Parser *parser, char *string);
bool ParserHas(Parser *parser, char *string);
void ParserList(Parser *parser, Block *parent, Block *(*ParserFunction)(Parser *parser, Block *parent, int32_t childI), int32_t startI, char *end, char *separator);

Block *ParserParseDo(Parser *parser, Block *parent, int32_t childI);
Block *ParserParseCase(Parser *parser, Block *parent, int32_t childI);
Block *ParserParseIfCases(Parser *parser, Block *parent, int32_t childI);
Block *ParserParseElseCase(Parser *parser, Block *parent, int32_t childI);
Block *ParserParseIf(Parser *parser, Block *parent, int32_t childI);
Block *ParserParseStatementList(Parser *parser, Block *parent, int32_t childI);
Block *ParserParseFunctionHeader(Parser *parser, Block *parent, int32_t childI);
Block *ParserParseFunction(Parser *parser, Block *parent, int32_t childI);
Block *ParserParseForLoop(Parser *parser, Block *parent, int32_t childI);
Block *ParserParseLambdaFunctionHeader(Parser *parser, Block *parent, int32_t childI);
Block *ParserParseLambdaFunction(Parser *parser, Block *parent, int32_t childI);
Block *ParserParseAddition(Parser *parser, Block *parent, int32_t childI);
Block *ParserParseUnarySuffix(Parser *parser, Block *parent, int32_t childI);
Block *ParserParsePrimary(Parser *parser, Block *parent, int32_t childI);

Block *ParserParseExpression(Parser *parser, Block *parent, int32_t childI);
Block *ParserParseStatement(Parser *parser, Block *parent, int32_t childI);
Block *ParserParseIdentifier(Parser *parser, Block *parent, int32_t childI);