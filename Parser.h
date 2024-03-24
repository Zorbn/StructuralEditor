#pragma once

#include "Lexer.h"
#include "Block.h"

typedef struct Parser
{
    Lexer lexer;
} Parser;

Parser ParserNew(Lexer lexer);

void ParserMatch(Parser *parser, char *string);
bool ParserHas(Parser *parser, char *string);
void ParserList(Parser *parser, Block *parent, Block *(*ParserFunction)(Parser *parser, Block *parent), int32_t startI, char *end, char *separator);

Block *ParserParseDo(Parser *parser, Block *parent);
Block *ParserParseCase(Parser *parser, Block *parent);
Block *ParserParseIfCases(Parser *parser, Block *parent);
Block *ParserParseElseCase(Parser *parser, Block *parent);
Block *ParserParseIf(Parser *parser, Block *parent);
Block *ParserParseStatementList(Parser *parser, Block *parent);
Block *ParserParseFunctionHeader(Parser *parser, Block *parent);
Block *ParserParseFunction(Parser *parser, Block *parent);
Block *ParserParseLambdaFunctionHeader(Parser *parser, Block *parent);
Block *ParserParseLambdaFunction(Parser *parser, Block *parent);
Block *ParserParseAddition(Parser *parser, Block *parent);
Block *ParserParseUnarySuffix(Parser *parser, Block *parent);
Block *ParserParsePrimary(Parser *parser, Block *parent);

Block *ParserParseExpression(Parser *parser, Block *parent);
Block *ParserParseStatement(Parser *parser, Block *parent);
Block *ParserParseIdentifier(Parser *parser, Block *parent);