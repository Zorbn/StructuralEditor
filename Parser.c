#include "Parser.h"

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Parser ParserNew(Lexer lexer)
{
    return (Parser){
        .lexer = lexer,
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

void ParserList(Parser *parser, Block **parent, Block *(*ParserFunction)(Parser *parser, Block *parent), int32_t startI, char *end, char *separator)
{
    int32_t i = startI;

    while (!ParserHas(parser, end))
    {
        if (separator && i > startI)
        {
            ParserMatch(parser, separator);
        }

        BlockReplaceChild(parent, ParserFunction(parser, *parent), i);
        i += 1;
    }

    LexerNext(&parser->lexer);
}

// Specific parse functions:

Block *ParserParseDo(Parser *parser, Block *parent)
{
    ParserMatch(parser, "do");

    Block *doBlock = BlockNew(BlockKindIdDo, parent);

    int32_t childI = 0;
    while (!ParserHas(parser, "end"))
    {
        Block *statement = ParserParseStatement(parser, doBlock);
        BlockReplaceChild(&doBlock, statement, childI);
        childI += 1;
    }

    return doBlock;
}

Block *ParserParseCase(Parser *parser, Block *parent)
{
    Block *caseBlock = BlockNew(BlockKindIdCase, parent);
    Block *condition = ParserParseExpression(parser, caseBlock);
    BlockReplaceChild(&caseBlock, condition, 1);
    ParserMatch(parser, "then");

    int32_t i = 1;
    while (!ParserHas(parser, "elseif") && !ParserHas(parser, "else") && !ParserHas(parser, "end"))
    {
        BlockReplaceChild(&caseBlock, ParserParseStatement(parser, caseBlock), i);
        i += 1;
    }

    return caseBlock;
}

Block *ParserParseIfCases(Parser *parser, Block *parent)
{
    Block *ifCases = BlockNew(BlockKindIdIfCases, parent);

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

        BlockReplaceChild(&ifCases, ParserParseCase(parser, ifCases), i);
        i += 1;
    }

    return ifCases;
}

Block *ParserParseElseCase(Parser *parser, Block *parent)
{
    ParserMatch(parser, "else");

    Block *elseCase = BlockNew(BlockKindIdElseCase, parent);

    ParserList(parser, &elseCase, ParserParseStatement, 0, "end", NULL);

    return elseCase;
}

Block *ParserParseIf(Parser *parser, Block *parent)
{
    Block *ifBlock = BlockNew(BlockKindIdIf, parent);

    BlockReplaceChild(&ifBlock, ParserParseIfCases(parser, ifBlock), 0);

    if (ParserHas(parser, "else"))
    {
        BlockReplaceChild(&ifBlock, ParserParseElseCase(parser, ifBlock), 1);
    }
    else
    {
        ParserMatch(parser, "end");
    }

    return ifBlock;
}

Block *ParserParseStatementList(Parser *parser, Block *parent)
{
    Block *statementList = BlockNew(BlockKindIdStatementList, parent);

    ParserList(parser, &statementList, ParserParseStatement, 0, "end", NULL);

    return statementList;
}

Block *ParserParseFunctionHeader(Parser *parser, Block *parent)
{
    Block *functionHeader = BlockNew(BlockKindIdFunctionHeader, parent);

    ParserMatch(parser, "function");

    BlockReplaceChild(&functionHeader, ParserParseIdentifier(parser, parent), 0);

    ParserMatch(parser, "(");
    ParserList(parser, &functionHeader, ParserParseIdentifier, 1, ")", ",");

    return functionHeader;
}

Block *ParserParseFunction(Parser *parser, Block *parent)
{
    Block *functionBlock = BlockNew(BlockKindIdFunction, parent);

    BlockReplaceChild(&functionBlock, ParserParseFunctionHeader(parser, functionBlock), 0);
    BlockReplaceChild(&functionBlock, ParserParseStatementList(parser, functionBlock), 1);

    return functionBlock;
}

Block *ParserParseLambdaFunctionHeader(Parser *parser, Block *parent)
{
    Block *lambdaFunctionHeader = BlockNew(BlockKindIdLambdaFunctionHeader, parent);

    ParserMatch(parser, "function");

    ParserMatch(parser, "(");
    ParserList(parser, &lambdaFunctionHeader, ParserParseIdentifier, 1, ")", ",");

    return lambdaFunctionHeader;
}

Block *ParserParseLambdaFunction(Parser *parser, Block *parent)
{
    Block *lambdaFunction = BlockNew(BlockKindIdLambdaFunction, parent);

    BlockReplaceChild(&lambdaFunction, ParserParseLambdaFunctionHeader(parser, lambdaFunction), 0);
    BlockReplaceChild(&lambdaFunction, ParserParseStatementList(parser, lambdaFunction), 1);

    return lambdaFunction;
}

Block *ParserParseAddition(Parser *parser, Block *parent)
{
    Block *left = ParserParseUnarySuffix(parser, parent);

    if (!ParserHas(parser, "+"))
    {
        return left;
    }

    Block *add = BlockNew(BlockKindIdAdd, parent);
    left->parent = add;
    BlockReplaceChild(&add, left, 0);

    int32_t childI = 1;
    while (ParserHas(parser, "+"))
    {
        LexerNext(&parser->lexer);

        Block *expression = ParserParseUnarySuffix(parser, add);
        BlockReplaceChild(&add, expression, childI);
        childI += 1;
    }

    return add;
}

Block *ParserParseUnarySuffix(Parser *parser, Block *parent)
{
    Block *left = ParserParsePrimary(parser, parent);

    while (ParserHas(parser, "("))
    {
        // This is a call.
        LexerNext(&parser->lexer);

        Block *call = BlockNew(BlockKindIdCall, parent);
        left->parent = call;
        BlockReplaceChild(&call, left, 0);

        int32_t childI = 1;
        while (!ParserHas(parser, ")"))
        {
            Block *expression = ParserParseExpression(parser, call);
            BlockReplaceChild(&call, expression, childI);
            childI += 1;

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

Block *ParserParsePrimary(Parser *parser, Block *parent)
{
    if (ParserHas(parser, "function"))
    {
        return ParserParseLambdaFunction(parser, parent);
    }

    return ParserParseIdentifier(parser, parent);
}

// Pin kinds:

Block *ParserParseExpression(Parser *parser, Block *parent)
{
    return ParserParseAddition(parser, parent);
}

Block *ParserParseStatement(Parser *parser, Block *parent)
{
    Token start = LexerPeek(&parser->lexer);

    if (LexerTokenEquals(&parser->lexer, start, "do"))
    {
        return ParserParseDo(parser, parent);
    }
    else if (LexerTokenEquals(&parser->lexer, start, "if"))
    {
        return ParserParseIf(parser, parent);
    }
    else if (LexerTokenEquals(&parser->lexer, start, "function"))
    {
        return ParserParseFunction(parser, parent);
    }

    Block *expression = ParserParseExpression(parser, parent);

    if (!ParserHas(parser, "="))
    {
        return expression;
    }

    LexerNext(&parser->lexer);

    Block *assignment = BlockNew(BlockKindIdAssignment, parent);
    expression->parent = assignment;
    Block *rightExpression = ParserParseExpression(parser, assignment);
    BlockReplaceChild(&assignment, expression, 0);
    BlockReplaceChild(&assignment, rightExpression, 1);

    return assignment;
}

// TODO: Simplify identifiers, ie: table.field should not be an identifier, it should be (. table field) where table and field are separate identifiers.
Block *ParserParseIdentifier(Parser *parser, Block *parent)
{
    Token text = LexerNext(&parser->lexer);

    Block *block = BlockNew(BlockKindIdIdentifier, parent);
    int32_t textLength = text.end - text.start;
    block->text = malloc(textLength + 1);
    strncpy(block->text, parser->lexer.data + text.start, textLength);
    block->text[textLength] = '\0';

    // TODO: Update block text size.

    return block;
}