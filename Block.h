#pragma once

#include "Font.h"
#include "Theme.h"
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

typedef struct Block Block;

typedef struct BlockParentData
{
    int32_t *children;
    int32_t childrenCount;
    int32_t childrenCapacity;
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
    int32_t parentI;

    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;

    BlockKindId kindId;
} Block;

ListDefine(Block)

// TODO: NO GLOBALS! (I was lazy here, I just want to test if this works before commiting more time to it).
extern List_Block blockPool;
extern uint64_t *blockPoolUsedBitMasks;
extern uint64_t blockPoolUsedBitMasksCount;

void BlockKindsInit(void);
void BlockKindsDeinit(void);
void BlockKindsUpdateTextSize(Font *font);

int32_t BlockNew(BlockKindId kindId, int32_t parentI, int32_t childI);
void BlockDelete(int32_t blockI);
int32_t BlockGetChildrenCount(int32_t blockI);
char *BlockGetText(int32_t blockI);
void BlockGetTextSize(int32_t blockI, int32_t *width, int32_t *height);
void BlockReplaceChild(int32_t blockI, int32_t childI, int32_t i);
uint64_t BlockCountAll(int32_t blockI);
void BlockUpdateTree(int32_t blockI, int32_t x, int32_t y);
void BlockDraw(
    int32_t blockI, int32_t cursorBlockI, int32_t depth, int32_t minY, int32_t maxY, Font *font, Theme *theme);