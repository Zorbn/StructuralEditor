#pragma once

#include "Font.h"
#include "List.h"

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
    BlockKindIdPin,
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

    BlockKindIdCount,
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
    int32_t textWidth;
    int32_t textHeight;
    bool isVertical;
    bool isGrowable;
    bool isTextInfix;
    DefaultChildKind *defaultChildren;
    int32_t defaultChildrenCount;
} BlockKind;

BlockKind BlockKinds[BlockKindIdCount];

typedef struct Block
{
    struct Block *parent;
    struct Block **children;
    char *text;

    // TODO: Use a union so that identifiers don't have child-related vars, and everything
    // else doesn't have text vars (char *text, textWidth, textHeight).
    int32_t childrenCount;
    int32_t childrenCapacity;

    // int16_t textWidth;
    // int16_t textHeight;

    // The element for this block in it's parent's children array.
    int32_t childI;

    BlockKindId kindId;
} Block;

typedef struct DrawCommand
{
    Block *block;

    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
} DrawCommand;

ListDefine(DrawCommand);

typedef struct DrawResult
{
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
} DrawResult;

void BlockKindsInit(Font *font);
void BlockKindsDeinit(void);

Block *BlockNew(BlockKindId kindId, Block *parent, int32_t childI);
void BlockDelete(Block *block);
void BlockReplaceChild(Block *block, Block *child, int32_t i);
uint64_t BlockCountAll(Block *block);
uint64_t BlockSizeAll(Block *block);
DrawResult BlockDraw(
    Block *block, int32_t x, int32_t y, int32_t minY, int32_t maxY, Font *font, List_DrawCommand *drawCommands);

void DrawCommandHandle(List_DrawCommand *drawCommands, Font *font);