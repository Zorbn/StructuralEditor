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

// TODO: Create a blockId type to distinguish between parentI here which is an index in blockPool and childI here which is an index in parent.children.
void ParserList(Parser *parser, int32_t parentI, int32_t (*ParserFunction)(Parser *parser, int32_t parentI, int32_t childI), int32_t startI, char *end, char *separator)
{
    int32_t i = startI;

    while (!ParserHas(parser, end))
    {
        if (separator && i > startI)
        {
            ParserMatch(parser, separator);
        }

        BlockReplaceChild(parentI, ParserFunction(parser, parentI, i), i);
        i += 1;
    }

    LexerNext(&parser->lexer);
}

// Specific parse functions:

int32_t ParserParseDo(Parser *parser, int32_t parentI, int32_t childI)
{
    ParserMatch(parser, "do");

    int32_t doBlockI = BlockNew(BlockKindIdDo, parentI, childI);

    int32_t i = 0;
    while (!ParserHas(parser, "end"))
    {
        int32_t statementI = ParserParseStatement(parser, doBlockI, i);
        BlockReplaceChild(doBlockI, statementI, i);
        i += 1;
    }

    return doBlockI;
}

int32_t ParserParseCase(Parser *parser, int32_t parentI, int32_t childI)
{
    int32_t caseBlockI = BlockNew(BlockKindIdCase, parentI, childI);
    int32_t conditionI = ParserParseExpression(parser, caseBlockI, 1);
    BlockReplaceChild(caseBlockI, conditionI, 1);
    ParserMatch(parser, "then");

    int32_t i = 1;
    while (!ParserHas(parser, "elseif") && !ParserHas(parser, "else") && !ParserHas(parser, "end"))
    {
        BlockReplaceChild(caseBlockI, ParserParseStatement(parser, caseBlockI, i), i);
        i += 1;
    }

    return caseBlockI;
}

int32_t ParserParseIfCases(Parser *parser, int32_t parentI, int32_t childI)
{
    int32_t ifCasesI = BlockNew(BlockKindIdIfCases, parentI, childI);

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

        BlockReplaceChild(ifCasesI, ParserParseCase(parser, ifCasesI, i), i);
        i += 1;
    }

    return ifCasesI;
}

int32_t ParserParseElseCase(Parser *parser, int32_t parentI, int32_t childI)
{
    ParserMatch(parser, "else");

    int32_t elseCaseI = BlockNew(BlockKindIdElseCase, parentI, childI);

    ParserList(parser, elseCaseI, ParserParseStatement, 0, "end", NULL);

    return elseCaseI;
}

int32_t ParserParseIf(Parser *parser, int32_t parentI, int32_t childI)
{
    int32_t ifBlockI = BlockNew(BlockKindIdIf, parentI, childI);

    BlockReplaceChild(ifBlockI, ParserParseIfCases(parser, ifBlockI, 0), 0);

    if (ParserHas(parser, "else"))
    {
        BlockReplaceChild(ifBlockI, ParserParseElseCase(parser, ifBlockI, 0), 1);
    }
    else
    {
        ParserMatch(parser, "end");
    }

    return ifBlockI;
}

int32_t ParserParseStatementList(Parser *parser, int32_t parentI, int32_t childI)
{
    int32_t statementListI = BlockNew(BlockKindIdStatementList, parentI, childI);

    ParserList(parser, statementListI, ParserParseStatement, 0, "end", NULL);

    return statementListI;
}

int32_t ParserParseFunctionHeader(Parser *parser, int32_t parentI, int32_t childI)
{
    int32_t functionHeaderI = BlockNew(BlockKindIdFunctionHeader, parentI, childI);

    ParserMatch(parser, "function");

    BlockReplaceChild(functionHeaderI, ParserParseIdentifier(parser, parentI, 0), 0);

    ParserMatch(parser, "(");
    ParserList(parser, functionHeaderI, ParserParseIdentifier, 1, ")", ",");

    return functionHeaderI;
}

int32_t ParserParseFunction(Parser *parser, int32_t parentI, int32_t childI)
{
    int32_t functionBlockI = BlockNew(BlockKindIdFunction, parentI, childI);

    BlockReplaceChild(functionBlockI, ParserParseFunctionHeader(parser, functionBlockI, 0), 0);
    BlockReplaceChild(functionBlockI, ParserParseStatementList(parser, functionBlockI, 1), 1);

    return functionBlockI;
}

int32_t ParserParseLambdaFunctionHeader(Parser *parser, int32_t parentI, int32_t childI)
{
    int32_t lambdaFunctionHeaderI = BlockNew(BlockKindIdLambdaFunctionHeader, parentI, childI);

    ParserMatch(parser, "function");

    ParserMatch(parser, "(");
    ParserList(parser, lambdaFunctionHeaderI, ParserParseIdentifier, 1, ")", ",");

    return lambdaFunctionHeaderI;
}

int32_t ParserParseLambdaFunction(Parser *parser, int32_t parentI, int32_t childI)
{
    int32_t lambdaFunctionI = BlockNew(BlockKindIdLambdaFunction, parentI, childI);

    BlockReplaceChild(lambdaFunctionI, ParserParseLambdaFunctionHeader(parser, lambdaFunctionI, 0), 0);
    BlockReplaceChild(lambdaFunctionI, ParserParseStatementList(parser, lambdaFunctionI, 1), 1);

    return lambdaFunctionI;
}

int32_t ParserParseAddition(Parser *parser, int32_t parentI, int32_t childI)
{
    int32_t leftI = ParserParseUnarySuffix(parser, parentI, childI);

    if (!ParserHas(parser, "+"))
    {
        return leftI;
    }

    int32_t addI = BlockNew(BlockKindIdAdd, parentI, childI);
    blockPool.data[leftI].parentI = addI;
    BlockReplaceChild(addI, leftI, 0);

    int32_t i = 1;
    while (ParserHas(parser, "+"))
    {
        LexerNext(&parser->lexer);

        int32_t expressionI = ParserParseUnarySuffix(parser, addI, i);
        BlockReplaceChild(addI, expressionI, i);
        i += 1;
    }

    return addI;
}

int32_t ParserParseUnarySuffix(Parser *parser, int32_t parentI, int32_t childI)
{
    int32_t leftI = ParserParsePrimary(parser, parentI, childI);

    while (ParserHas(parser, "("))
    {
        // This is a call.
        LexerNext(&parser->lexer);

        int32_t callI = BlockNew(BlockKindIdCall, parentI, childI);
        blockPool.data[leftI].parentI = callI;
        BlockReplaceChild(callI, leftI, 0);

        int32_t i = 1;
        while (!ParserHas(parser, ")"))
        {
            int32_t expressionI = ParserParseExpression(parser, callI, i);
            BlockReplaceChild(callI, expressionI, i);
            i += 1;

            if (!ParserHas(parser, ","))
            {
                break;
            }

            LexerNext(&parser->lexer);
        }

        ParserMatch(parser, ")");

        leftI = callI;
    }

    return leftI;
}

int32_t ParserParsePrimary(Parser *parser, int32_t parentI, int32_t childI)
{
    if (ParserHas(parser, "function"))
    {
        return ParserParseLambdaFunction(parser, parentI, childI);
    }

    return ParserParseIdentifier(parser, parentI, childI);
}

// Pin kinds:

int32_t ParserParseExpression(Parser *parser, int32_t parentI, int32_t childI)
{
    return ParserParseAddition(parser, parentI, childI);
}

int32_t ParserParseStatement(Parser *parser, int32_t parentI, int32_t childI)
{
    Token start = LexerPeek(&parser->lexer);

    if (LexerTokenEquals(&parser->lexer, start, "do"))
    {
        return ParserParseDo(parser, parentI, childI);
    }
    else if (LexerTokenEquals(&parser->lexer, start, "if"))
    {
        return ParserParseIf(parser, parentI, childI);
    }
    else if (LexerTokenEquals(&parser->lexer, start, "function"))
    {
        return ParserParseFunction(parser, parentI, childI);
    }

    int32_t expressionI = ParserParseExpression(parser, parentI, childI);

    if (!ParserHas(parser, "="))
    {
        return expressionI;
    }

    LexerNext(&parser->lexer);

    int32_t assignmentI = BlockNew(BlockKindIdAssignment, parentI, childI);
    blockPool.data[expressionI].parentI = assignmentI;
    int32_t rightExpressionI = ParserParseExpression(parser, assignmentI, 1);
    BlockReplaceChild(assignmentI, expressionI, 0);
    BlockReplaceChild(assignmentI, rightExpressionI, 1);

    return assignmentI;
}

// TODO: Simplify identifiers, ie: table.field should not be an identifier, it should be (. table field) where table and field are separate identifiers.
int32_t ParserParseIdentifier(Parser *parser, int32_t parentI, int32_t childI)
{
    Token text = LexerNext(&parser->lexer);

    int32_t blockI = BlockNew(BlockKindIdIdentifier, parentI, childI);
    BlockIdentifierData *identifierData = &blockPool.data[blockI].data.identifier;

    int32_t textLength = text.end - text.start;
    identifierData->text = malloc(textLength + 1);
    strncpy(identifierData->text, parser->lexer.data + text.start, textLength);
    identifierData->text[textLength] = '\0';

    GetTextSize(identifierData->text, &identifierData->textWidth, &identifierData->textHeight, parser->font);

    return blockI;
}