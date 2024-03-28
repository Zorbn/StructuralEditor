#include "Parser.h"

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Parser ParserNew(Lexer lexer, Font *font)
{
    return (Parser){
        .lexer = lexer,
        .font = font,
    };
}

void ParserMatch(Parser *parser, char *string)
{
    Token next = LexerNext(&parser->lexer);

    if (!LexerTokenEquals(&parser->lexer, next, string))
    {
        fprintf(stderr, "Expected %s!\n", string);
        exit(EXIT_FAILURE);
    }
}

bool ParserHas(Parser *parser, char *string)
{
    return LexerTokenEquals(&parser->lexer, parser->lexer.current, string);
}

void ParserList(Parser *parser, Block *parent, Block *(*ParserFunction)(Parser *parser, Block *parent, int32_t childI), int32_t startI, char *end, char *separator)
{
    int32_t i = startI;

    while (!ParserHas(parser, end))
    {
        if (separator && i > startI)
        {
            ParserMatch(parser, separator);
        }

        BlockReplaceChild(parent, ParserFunction(parser, parent, i), i);
        i += 1;
    }

    LexerNext(&parser->lexer);
}

// Specific parse functions:

Block *ParserParseDo(Parser *parser, Block *parent, int32_t childI)
{
    ParserMatch(parser, "do");

    Block *doBlock = BlockNew(BlockKindIdDo, parent, childI);

    int32_t i = 0;
    while (!ParserHas(parser, "end"))
    {
        Block *statement = ParserParseStatement(parser, doBlock, i);
        BlockReplaceChild(doBlock, statement, i);
        i += 1;
    }

    return doBlock;
}

Block *ParserParseCase(Parser *parser, Block *parent, int32_t childI)
{
    Block *caseBlock = BlockNew(BlockKindIdCase, parent, childI);
    Block *condition = ParserParseExpression(parser, caseBlock, 0);
    BlockReplaceChild(caseBlock, condition, 0);
    ParserMatch(parser, "then");

    int32_t i = 1;
    while (!ParserHas(parser, "elseif") && !ParserHas(parser, "else") && !ParserHas(parser, "end"))
    {
        BlockReplaceChild(caseBlock, ParserParseStatement(parser, caseBlock, i), i);
        i += 1;
    }

    return caseBlock;
}

Block *ParserParseIfCases(Parser *parser, Block *parent, int32_t childI)
{
    Block *ifCases = BlockNew(BlockKindIdIfCases, parent,childI);

    int32_t i = 0;
    while (!ParserHas(parser, "else") && !ParserHas(parser, "end"))
    {
        if (i == 0)
        {
            ParserMatch(parser, "if");
        }
        else
        {
            ParserMatch(parser, "elseif");
        }

        BlockReplaceChild(ifCases, ParserParseCase(parser, ifCases, i), i);
        i += 1;
    }

    return ifCases;
}

Block *ParserParseElseCase(Parser *parser, Block *parent, int32_t childI)
{
    ParserMatch(parser, "else");

    Block *elseCase = BlockNew(BlockKindIdElseCase, parent, childI);

    ParserList(parser, elseCase, ParserParseStatement, 0, "end", NULL);

    return elseCase;
}

Block *ParserParseIf(Parser *parser, Block *parent, int32_t childI)
{
    Block *ifBlock = BlockNew(BlockKindIdIf, parent, childI);

    BlockReplaceChild(ifBlock, ParserParseIfCases(parser, ifBlock, 0), 0);

    if (ParserHas(parser, "else"))
    {
        BlockReplaceChild(ifBlock, ParserParseElseCase(parser, ifBlock, 0), 1);
    }
    else
    {
        ParserMatch(parser, "end");
    }

    return ifBlock;
}

Block *ParserParseStatementList(Parser *parser, Block *parent, int32_t childI)
{
    Block *statementList = BlockNew(BlockKindIdStatementList, parent, childI);

    ParserList(parser, statementList, ParserParseStatement, 0, "end", NULL);

    return statementList;
}

Block *ParserParseFunctionHeader(Parser *parser, Block *parent, int32_t childI)
{
    Block *functionHeader = BlockNew(BlockKindIdFunctionHeader, parent, childI);

    ParserMatch(parser, "function");

    BlockReplaceChild(functionHeader, ParserParseIdentifier(parser, parent, 0), 0);

    ParserMatch(parser, "(");
    ParserList(parser, functionHeader, ParserParseIdentifier, 1, ")", ",");

    return functionHeader;
}

Block *ParserParseFunction(Parser *parser, Block *parent, int32_t childI)
{
    Block *functionBlock = BlockNew(BlockKindIdFunction, parent, childI);

    BlockReplaceChild(functionBlock, ParserParseFunctionHeader(parser, functionBlock, 0), 0);
    BlockReplaceChild(functionBlock, ParserParseStatementList(parser, functionBlock, 1), 1);

    return functionBlock;
}

Block *ParserParseLambdaFunctionHeader(Parser *parser, Block *parent, int32_t childI)
{
    Block *lambdaFunctionHeader = BlockNew(BlockKindIdLambdaFunctionHeader, parent, childI);

    ParserMatch(parser, "function");

    ParserMatch(parser, "(");
    ParserList(parser, lambdaFunctionHeader, ParserParseIdentifier, 1, ")", ",");

    return lambdaFunctionHeader;
}

Block *ParserParseLambdaFunction(Parser *parser, Block *parent, int32_t childI)
{
    Block *lambdaFunction = BlockNew(BlockKindIdLambdaFunction, parent, childI);

    BlockReplaceChild(lambdaFunction, ParserParseLambdaFunctionHeader(parser, lambdaFunction, 0), 0);
    BlockReplaceChild(lambdaFunction, ParserParseStatementList(parser, lambdaFunction, 1), 1);

    return lambdaFunction;
}

Block *ParserParseAddition(Parser *parser, Block *parent, int32_t childI)
{
    Block *left = ParserParseUnarySuffix(parser, parent, childI);

    if (!ParserHas(parser, "+"))
    {
        return left;
    }

    Block *add = BlockNew(BlockKindIdAdd, parent, childI);
    left->parent = add;
    BlockReplaceChild(add, left, 0);

    int32_t i = 1;
    while (ParserHas(parser, "+"))
    {
        LexerNext(&parser->lexer);

        Block *expression = ParserParseUnarySuffix(parser, add, i);
        BlockReplaceChild(add, expression, i);
        i += 1;
    }

    return add;
}

Block *ParserParseUnarySuffix(Parser *parser, Block *parent, int32_t childI)
{
    Block *left = ParserParsePrimary(parser, parent, childI);

    while (ParserHas(parser, "("))
    {
        // This is a call.
        LexerNext(&parser->lexer);

        Block *call = BlockNew(BlockKindIdCall, parent, childI);
        left->parent = call;
        BlockReplaceChild(call, left, 0);

        int32_t i = 1;
        while (!ParserHas(parser, ")"))
        {
            Block *expression = ParserParseExpression(parser, call, i);
            BlockReplaceChild(call, expression, i);
            i += 1;

            if (!ParserHas(parser, ","))
            {
                break;
            }

            LexerNext(&parser->lexer);
        }

        ParserMatch(parser, ")");

        left = call;
    }

    return left;
}

Block *ParserParsePrimary(Parser *parser, Block *parent, int32_t childI)
{
    if (ParserHas(parser, "function"))
    {
        return ParserParseLambdaFunction(parser, parent, childI);
    }

    return ParserParseIdentifier(parser, parent, childI);
}

// Pin kinds:

Block *ParserParseExpression(Parser *parser, Block *parent, int32_t childI)
{
    return ParserParseAddition(parser, parent, childI);
}

Block *ParserParseStatement(Parser *parser, Block *parent, int32_t childI)
{
    Token start = LexerPeek(&parser->lexer);

    if (LexerTokenEquals(&parser->lexer, start, "do"))
    {
        return ParserParseDo(parser, parent, childI);
    }
    else if (LexerTokenEquals(&parser->lexer, start, "if"))
    {
        return ParserParseIf(parser, parent, childI);
    }
    else if (LexerTokenEquals(&parser->lexer, start, "function"))
    {
        return ParserParseFunction(parser, parent, childI);
    }

    Block *expression = ParserParseExpression(parser, parent, childI);

    if (!ParserHas(parser, "="))
    {
        return expression;
    }

    LexerNext(&parser->lexer);

    Block *assignment = BlockNew(BlockKindIdAssignment, parent, childI);
    expression->parent = assignment;
    Block *rightExpression = ParserParseExpression(parser, assignment, 1);
    BlockReplaceChild(assignment, expression, 0);
    BlockReplaceChild(assignment, rightExpression, 1);

    return assignment;
}

// TODO: Simplify identifiers, ie: table.field should not be an identifier, it should be (. table field) where table and field are separate identifiers.
Block *ParserParseIdentifier(Parser *parser, Block *parent, int32_t childI)
{
    Token text = LexerNext(&parser->lexer);

    Block *block = BlockNew(BlockKindIdIdentifier, parent, childI);
    BlockIdentifierData *identifierData = &block->data.identifier;

    int32_t textLength = text.end - text.start;
    identifierData->text = malloc(textLength + 1);
    strncpy(identifierData->text, parser->lexer.data + text.start, textLength);
    identifierData->text[textLength] = '\0';

    GetTextSize(identifierData->text, &identifierData->textWidth, &identifierData->textHeight, parser->font);

    return block;
}