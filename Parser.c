#include "Parser.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Parser ParserNew(Lexer lexer, Font *font)
{
    return (Parser){
        .lexer = lexer,
        .font = font,
        .textBuffer = ListNew_char(16),
    };
}

void ParserDelete(Parser *parser)
{
    ListDelete_char(&parser->textBuffer);
}

void ParserMatch(Parser *parser, char *string)
{
    Token next = LexerNext(&parser->lexer);

    if (!LexerTokenEquals(&parser->lexer, next, string))
    {
        fprintf(stderr, "Expected \"%s\" but got: ", string);

        for (int32_t i = 0; i < next.end - next.start; i++)
        {
            fprintf(stderr, "%c", parser->lexer.data[next.start + i]);
        }

        fprintf(stderr, "\n");

        assert(false);
        exit(EXIT_FAILURE);
    }
}

bool ParserHas(Parser *parser, char *string)
{
    return LexerTokenEquals(&parser->lexer, parser->lexer.current, string);
}

void ParserList(Parser *parser, Block *parent, Block *(*ParserFunction)(Parser *parser, Block *parent, int32_t childI),
    int32_t startI, char *end, char *separator)
{
    int32_t i = startI;

    while (!ParserHas(parser, end))
    {
        if (separator && i > startI)
        {
            ParserMatch(parser, separator);
        }

        BlockReplaceChild(parent, ParserFunction(parser, parent, i), i, true);
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
        BlockReplaceChild(doBlock, statement, i, true);
        i += 1;
    }

    ParserMatch(parser, "end");

    return doBlock;
}

Block *ParserParseCase(Parser *parser, Block *parent, int32_t childI)
{
    Block *caseBlock = BlockNew(BlockKindIdCase, parent, childI);
    Block *condition = ParserParseExpression(parser, caseBlock, 0);
    BlockReplaceChild(caseBlock, condition, 0, true);
    ParserMatch(parser, "then");

    int32_t i = 1;
    while (!ParserHas(parser, "elseif") && !ParserHas(parser, "else") && !ParserHas(parser, "end"))
    {
        BlockReplaceChild(caseBlock, ParserParseStatement(parser, caseBlock, i), i, true);
        i += 1;
    }

    return caseBlock;
}

Block *ParserParseIfCases(Parser *parser, Block *parent, int32_t childI)
{
    Block *ifCases = BlockNew(BlockKindIdIfCases, parent, childI);

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

        BlockReplaceChild(ifCases, ParserParseCase(parser, ifCases, i), i, true);
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

    BlockReplaceChild(ifBlock, ParserParseIfCases(parser, ifBlock, 0), 0, true);

    if (ParserHas(parser, "else"))
    {
        BlockReplaceChild(ifBlock, ParserParseElseCase(parser, ifBlock, 0), 1, true);
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

Block *ParserParseForLoop(Parser *parser, Block *parent, int32_t childI)
{
    ParserMatch(parser, "for");

    Block *forLoop = NULL;
    Block *iterator = ParserParseIdentifier(parser, parent, childI);

    if (ParserHas(parser, "="))
    {
        ParserMatch(parser, "=");

        forLoop = BlockNew(BlockKindIdForLoop, parent, childI);

        Block *forLoopCondition = BlockNew(BlockKindIdForLoopCondition, forLoop, 0);
        BlockReplaceChild(forLoop, forLoopCondition, 0, true);
        BlockReplaceChild(forLoopCondition, iterator, 0, true);

        Block *forLoopBounds = BlockNew(BlockKindIdForLoopBounds, forLoopCondition, 1);
        BlockReplaceChild(forLoopCondition, forLoopBounds, 1, true);

        Block *lowBound = ParserParseExpression(parser, forLoopCondition, 0);
        BlockReplaceChild(forLoopBounds, lowBound, 0, true);

        ParserMatch(parser, ",");

        Block *highBound = ParserParseExpression(parser, forLoopCondition, 1);
        BlockReplaceChild(forLoopBounds, highBound, 1, true);

        if (ParserHas(parser, ","))
        {
            ParserMatch(parser, ",");

            Block *step = ParserParseExpression(parser, forLoopCondition, 2);
            BlockReplaceChild(forLoopBounds, step, 2, true);
        }
    }
    else
    {
        ParserMatch(parser, "in");

        forLoop = BlockNew(BlockKindIdForInLoop, parent, childI);

        Block *forLoopCondition = BlockNew(BlockKindIdForInLoopCondition, forLoop, 0);
        BlockReplaceChild(forLoop, forLoopCondition, 0, true);
        BlockReplaceChild(forLoopCondition, iterator, 0, true);

        Block *iteratorFunction = ParserParseExpression(parser, forLoopCondition, 1);
        BlockReplaceChild(forLoopCondition, iteratorFunction, 1, true);
    }

    ParserMatch(parser, "do");

    Block *statementList = ParserParseStatementList(parser, forLoop, 1);
    BlockReplaceChild(forLoop, statementList, 1, true);

    return forLoop;
}

Block *ParserParseWhileLoop(Parser *parser, Block *parent, int32_t childI)
{
    ParserMatch(parser, "while");

    Block *whileLoop = BlockNew(BlockKindIdWhileLoop, parent, childI);

    BlockReplaceChild(whileLoop, ParserParseExpression(parser, whileLoop, 0), 0, true);

    ParserMatch(parser, "do");

    BlockReplaceChild(whileLoop, ParserParseStatementList(parser, whileLoop, 1), 1, true);

    return whileLoop;
}

Block *ParserParseFunctionHeader(Parser *parser, Block *parent, int32_t childI)
{
    Block *functionHeader = BlockNew(BlockKindIdFunctionHeader, parent, childI);

    ParserMatch(parser, "function");

    BlockReplaceChild(functionHeader, ParserParseIdentifier(parser, parent, 0), 0, true);

    ParserMatch(parser, "(");
    ParserList(parser, functionHeader, ParserParseIdentifier, 1, ")", ",");

    return functionHeader;
}

Block *ParserParseFunction(Parser *parser, Block *parent, int32_t childI)
{
    Block *functionBlock = BlockNew(BlockKindIdFunction, parent, childI);

    BlockReplaceChild(functionBlock, ParserParseFunctionHeader(parser, functionBlock, 0), 0, true);
    BlockReplaceChild(functionBlock, ParserParseStatementList(parser, functionBlock, 1), 1, true);

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

    BlockReplaceChild(lambdaFunction, ParserParseLambdaFunctionHeader(parser, lambdaFunction, 0), 0, true);
    BlockReplaceChild(lambdaFunction, ParserParseStatementList(parser, lambdaFunction, 1), 1, true);

    return lambdaFunction;
}

static Block *ParserParseBlockList(Parser *parser, Block *parent, int32_t childI, BlockKindId kindId, char *separator,
    Block *(*innerParseFunction)(Parser *parser, Block *parent, int32_t childI))
{
    Block *left = innerParseFunction(parser, parent, childI);

    if (!ParserHas(parser, separator))
    {
        return left;
    }

    Block *block = BlockNew(kindId, parent, childI);
    left->parent = block;
    BlockReplaceChild(block, left, 0, true);

    int32_t i = 1;
    while (ParserHas(parser, separator))
    {
        LexerNext(&parser->lexer);

        Block *expression = innerParseFunction(parser, block, i);
        BlockReplaceChild(block, expression, i, true);
        i += 1;
    }

    return block;
}

static Block *ParserParseBlockPair(Parser *parser, Block *parent, int32_t childI, BlockKindId kindId, char *separator,
    Block *(*innerParseFunction)(Parser *parser, Block *parent, int32_t childI))
{
    Block *left = innerParseFunction(parser, parent, childI);

    if (!ParserHas(parser, separator))
    {
        return left;
    }

    Block *block = BlockNew(kindId, parent, childI);

    BlockReplaceChild(block, left, 0, true);
    ParserMatch(parser, separator);
    BlockReplaceChild(block, innerParseFunction(parser, block, 1), 1, true);

    return block;
}

Block *ParserParseConcatenate(Parser *parser, Block *parent, int32_t childI)
{
    return ParserParseBlockList(parser, parent, childI, BlockKindIdConcatenate, "..", ParserParseUnaryPrefix);
}

Block *ParserParseModulo(Parser *parser, Block *parent, int32_t childI)
{
    return ParserParseBlockList(parser, parent, childI, BlockKindIdModulo, "%", ParserParseConcatenate);
}

Block *ParserParseDivide(Parser *parser, Block *parent, int32_t childI)
{
    return ParserParseBlockList(parser, parent, childI, BlockKindIdDivide, "/", ParserParseModulo);
}

Block *ParserParseMultiply(Parser *parser, Block *parent, int32_t childI)
{
    return ParserParseBlockList(parser, parent, childI, BlockKindIdMultiply, "*", ParserParseDivide);
}

Block *ParserParseSubtract(Parser *parser, Block *parent, int32_t childI)
{
    return ParserParseBlockList(parser, parent, childI, BlockKindIdSubtract, "-", ParserParseMultiply);
}

Block *ParserParseAdd(Parser *parser, Block *parent, int32_t childI)
{
    return ParserParseBlockList(parser, parent, childI, BlockKindIdAdd, "+", ParserParseSubtract);
}

Block *ParserParseGreaterEqual(Parser *parser, Block *parent, int32_t childI)
{
    return ParserParseBlockPair(parser, parent, childI, BlockKindIdGreaterEqual, ">=", ParserParseAdd);
}

Block *ParserParseLessEqual(Parser *parser, Block *parent, int32_t childI)
{
    return ParserParseBlockPair(parser, parent, childI, BlockKindIdLessEqual, "<=", ParserParseGreaterEqual);
}

Block *ParserParseGreater(Parser *parser, Block *parent, int32_t childI)
{
    return ParserParseBlockPair(parser, parent, childI, BlockKindIdGreater, ">", ParserParseLessEqual);
}

Block *ParserParseLess(Parser *parser, Block *parent, int32_t childI)
{
    return ParserParseBlockPair(parser, parent, childI, BlockKindIdLess, "<", ParserParseGreater);
}

Block *ParserParseNotEqual(Parser *parser, Block *parent, int32_t childI)
{
    return ParserParseBlockList(parser, parent, childI, BlockKindIdNotEqual, "!=", ParserParseLess);
}

Block *ParserParseEqual(Parser *parser, Block *parent, int32_t childI)
{
    return ParserParseBlockList(parser, parent, childI, BlockKindIdEqual, "==", ParserParseNotEqual);
}

Block *ParserParseAnd(Parser *parser, Block *parent, int32_t childI)
{
    return ParserParseBlockList(parser, parent, childI, BlockKindIdAnd, "and", ParserParseEqual);
}

Block *ParserParseOr(Parser *parser, Block *parent, int32_t childI)
{
    return ParserParseBlockList(parser, parent, childI, BlockKindIdOr, "or", ParserParseAnd);
}

Block *ParserParseUnaryPrefix(Parser *parser, Block *parent, int32_t childI)
{
    if (ParserHas(parser, "#"))
    {
        ParserMatch(parser, "#");

        Block *block = BlockNew(BlockKindIdLength, parent, childI);

        BlockReplaceChild(block, ParserParseUnaryPrefix(parser, block, 0), 0, true);

        return block;
    }
    else if (ParserHas(parser, "not"))
    {
        ParserMatch(parser, "not");

        Block *block = BlockNew(BlockKindIdNot, parent, childI);

        BlockReplaceChild(block, ParserParseUnaryPrefix(parser, block, 0), 0, true);

        return block;
    }

    return ParserParseUnarySuffix(parser, parent, childI);
}

Block *ParserParseUnarySuffix(Parser *parser, Block *parent, int32_t childI)
{
    if (ParserHas(parser, "("))
    {
        // This is a parenthesized expression.
        ParserMatch(parser, "(");

        Block *expression = ParserParseExpression(parser, parent, childI);

        ParserMatch(parser, ")");

        return expression;
    }

    Block *left = ParserParsePrimary(parser, parent, childI);

    while (ParserHas(parser, "("))
    {
        // This is a call.
        LexerNext(&parser->lexer);

        Block *call = BlockNew(BlockKindIdCall, parent, childI);
        left->parent = call;
        BlockReplaceChild(call, left, 0, true);

        int32_t i = 1;
        while (!ParserHas(parser, ")"))
        {
            Block *expression = ParserParseExpression(parser, call, i);
            BlockReplaceChild(call, expression, i, true);
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
    return ParserParseOr(parser, parent, childI);
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
    else if (LexerTokenEquals(&parser->lexer, start, "for"))
    {
        return ParserParseForLoop(parser, parent, childI);
    }
    else if (LexerTokenEquals(&parser->lexer, start, "while"))
    {
        return ParserParseWhileLoop(parser, parent, childI);
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
    BlockReplaceChild(assignment, expression, 0, true);
    BlockReplaceChild(assignment, rightExpression, 1, true);

    return assignment;
}

// TODO: Simplify identifiers, ie: table.field should not be an identifier, it should be (. table field) where table and
// field are separate identifiers.
Block *ParserParseIdentifier(Parser *parser, Block *parent, int32_t childI)
{
    Token text = LexerNext(&parser->lexer);
    int32_t textCount = text.end - text.start;

    ListReset_char(&parser->textBuffer);
    ListReserve_char(&parser->textBuffer, textCount);

    char firstChar = parser->lexer.data[text.start];
    bool doConvert = firstChar != '"' && firstChar != '\'';

    for (int32_t i = 0; i < textCount; i++)
    {
        char textChar = parser->lexer.data[text.start + i];

        if (doConvert && textChar == '_')
        {
            textChar = ' ';
        }

        ListPush_char(&parser->textBuffer, textChar);
    }

    Block *block = BlockNewIdentifier(parser->textBuffer.data, parser->textBuffer.count, parser->font, parent, childI);

    return block;
}