#pragma once

#include <inttypes.h>
#include <stdbool.h>

typedef enum PinKind
{
    PinKindExpression,
    PinKindStatement,
    PinKindIdentifier,
    PinKindNone,
} PinKind;

typedef enum BlockKindId
{
    BlockKindIdPin = 0,
    BlockKindIdDo,
    BlockKindIdStatementList,
    BlockKindIdFunctionHeader,
    BlockKindIdFunction,
    BlockKindIdLambdaFunctionHeader,
    BlockKindIdLambdaFunction,
    BlockKindIdCase,
    BlockKindIdIfCases,
    BlockKindIdElseCase,
    BlockKindIdIf,
    BlockKindIdAssignment,
    BlockKindIdAdd,
    BlockKindIdCall,
    BlockKindIdIdentifier,
} BlockKindId;

typedef struct DefaultChildKind
{
    bool isPin;
    PinKind pinKind;
    BlockKindId blockKindId;
} DefaultChildKind;

typedef struct BlockKind
{
    PinKind pinKind;
    char *searchText;
    char *text;
    bool isVertical;
    bool isGrowable;
    bool isTextInfix;
    DefaultChildKind *defaultChildren;
    int32_t defaultChildrenCount;
} BlockKind;

const BlockKind BlockKinds[];

typedef struct Block
{
    struct Block *parent;
    struct Block **children;
    char *text;

    int32_t childrenCount;
    int32_t childrenCapacity;

    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;

    int32_t textWidth;
    int32_t textHeight;

    BlockKindId kindId;
} Block;

Block *BlockNew(BlockKindId kindId, Block *parent);
void BlockDelete(Block *block);
void BlockReplaceChild(Block *block, Block *child, int64_t i);
int32_t BlockCountAll(Block *block);