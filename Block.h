#pragma once

#include "Font.h"
#include "Theme.h"
#include "List.h"
#include "Camera.h"

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

typedef struct PinKindInsertBlocks
{
    BlockKindId *blockKindIds;
    int32_t blockKindIdCount;
} PinKindInsertBlocks;

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

const PinKindInsertBlocks PinInsertBlocks[];
BlockKind BlockKinds[BlockKindIdCount];

typedef struct Block Block;

typedef Block *BlockPointer;
ListDefine(BlockPointer);

typedef struct BlockParentData
{
    List_BlockPointer children;
} BlockParentData;

typedef struct BlockIdentifierData
{
    char *text;
    int32_t textWidth;
    int32_t textHeight;
} BlockIdentifierData;

// A block can either be a parent or an identifier.
typedef union BlockData
{
    BlockParentData parent;
    BlockIdentifierData identifier;
} BlockData;

typedef struct Block
{
    BlockData data;
    Block *parent;

    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;

    int32_t childI;
    BlockKindId kindId;
} Block;

void BlockKindsInit(void);
void BlockKindsDeinit(void);
void BlockKindsUpdateTextSize(Font *font);

Block *BlockNew(BlockKindId kindId, Block *parent, int32_t childI);
Block *BlockNewIdentifier(char *text, int32_t textLength, Font *font, Block *parent, int32_t childI);
Block *BlockCopy(Block *other, Block *parent, int32_t childI);
void BlockDelete(Block *block);
void BlockMarkNeedsUpdate(Block *block);
int32_t BlockGetChildrenCount(Block *block);
char *BlockGetText(Block *block);
void BlockGetTextSize(Block *block, int32_t *width, int32_t *height);
DefaultChildKind *BlockGetDefaultChild(Block *block, int32_t childI);
void BlockGetGlobalPosition(Block *block, int32_t *x, int32_t *y);
bool BlockCanSwapWith(Block *block, DefaultChildKind *otherDefaultChildKind);
void BlockReplaceChild(Block *block, Block *child, int32_t childI);
void BlockInsertChild(Block *block, Block *child, int32_t childI);
uint64_t BlockCountAll(Block *block);
void BlockUpdateTree(Block *block, int32_t x, int32_t y);
void BlockDraw(Block *block, Block *cursorBlock, int32_t depth, Camera *camera, Font *font, Theme *theme, int32_t x, int32_t y);