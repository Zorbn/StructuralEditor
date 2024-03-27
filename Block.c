#include "Block.h"
#include "Math.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>

#define SOKOL_GLCORE33
#include <sokol_gfx.h>
#include <sokol_gp.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>

DefaultChildKind NewChild(BlockKindId childBlockKindId)
{
    return (DefaultChildKind){
        .blockKindId = childBlockKindId,
        .isPin = false,
    };
}

DefaultChildKind NewChildPin(PinKind childPinKind)
{
    return (DefaultChildKind){
        .pinKind = childPinKind,
        .isPin = true,
    };
}

BlockKind BlockKindNew(BlockKind blockKind)
{
    DefaultChildKind *defaultChildren = malloc(sizeof(DefaultChildKind) * blockKind.defaultChildrenCount);

    for (int32_t i = 0; i < blockKind.defaultChildrenCount; i++)
    {
        defaultChildren[i] = blockKind.defaultChildren[i];
    }

    blockKind.defaultChildren = defaultChildren;

    return blockKind;
}

BlockKind BlockKinds[BlockKindIdCount];

void BlockKindsInit(void)
{
    BlockKinds[BlockKindIdPin] = BlockKindNew((BlockKind){
        .pinKind = PinKindNone,
        .text = ".",
    });
    BlockKinds[BlockKindIdDo] = BlockKindNew((BlockKind){
        .pinKind = PinKindStatement,
        .searchText = "do",
        .text = "do",
        .isVertical = true,
        .isGrowable = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindStatement),
            },
        .defaultChildrenCount = 1,
    });
    BlockKinds[BlockKindIdStatementList] = BlockKindNew((BlockKind){
        .pinKind = PinKindNone,
        .isVertical = true,
        .isGrowable = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindStatement),
            },
        .defaultChildrenCount = 1,
    });
    BlockKinds[BlockKindIdFunctionHeader] = BlockKindNew((BlockKind){
        .pinKind = PinKindNone,
        .text = "fn",
        .isGrowable = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindIdentifier),
                NewChildPin(PinKindIdentifier),
            },
        .defaultChildrenCount = 2,
    });
    BlockKinds[BlockKindIdFunction] = BlockKindNew((BlockKind){
        .pinKind = PinKindStatement,
        .searchText = "fn",
        .isVertical = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChild(BlockKindIdFunctionHeader),
                NewChild(BlockKindIdStatementList),
            },
        .defaultChildrenCount = 2,
    });
    BlockKinds[BlockKindIdLambdaFunctionHeader] = BlockKindNew((BlockKind){
        .pinKind = PinKindNone,
        .text = "fn",
        .isGrowable = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindIdentifier),
            },
        .defaultChildrenCount = 1,
    });
    BlockKinds[BlockKindIdLambdaFunction] = BlockKindNew((BlockKind){
        .pinKind = PinKindExpression,
        .searchText = "fn",
        .isVertical = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChild(BlockKindIdLambdaFunctionHeader),
                NewChild(BlockKindIdStatementList),
            },
        .defaultChildrenCount = 2,
    });
    BlockKinds[BlockKindIdCase] = BlockKindNew((BlockKind){
        .pinKind = PinKindNone,
        .text = "case",
        .isVertical = true,
        .isGrowable = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindExpression),
                NewChildPin(PinKindStatement),
            },
        .defaultChildrenCount = 2,
    });
    BlockKinds[BlockKindIdIfCases] = BlockKindNew((BlockKind){
        .pinKind = PinKindNone,
        .text = "if",
        .isVertical = true,
        .isGrowable = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChild(BlockKindIdCase),
            },
        .defaultChildrenCount = 1,
    });
    BlockKinds[BlockKindIdElseCase] = BlockKindNew((BlockKind){
        .pinKind = PinKindNone,
        .text = "else",
        .isVertical = true,
        .isGrowable = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindStatement),
            },
        .defaultChildrenCount = 1,
    });
    BlockKinds[BlockKindIdIf] = BlockKindNew((BlockKind){
        .pinKind = PinKindStatement,
        .searchText = "if",
        .isVertical = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChild(BlockKindIdIfCases),
                NewChild(BlockKindIdElseCase),
            },
        .defaultChildrenCount = 2,
    });
    BlockKinds[BlockKindIdAssignment] = BlockKindNew((BlockKind){
        .pinKind = PinKindStatement,
        .searchText = "=",
        .text = "=",
        .isTextInfix = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindExpression),
                NewChildPin(PinKindExpression),
            },
        .defaultChildrenCount = 2,
    });
    BlockKinds[BlockKindIdAdd] = BlockKindNew((BlockKind){
        .pinKind = PinKindExpression,
        .searchText = "+",
        .text = "+",
        .isTextInfix = true,
        .isGrowable = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindExpression),
                NewChildPin(PinKindExpression),
                NewChildPin(PinKindExpression),
            },
        .defaultChildrenCount = 3,
    });
    BlockKinds[BlockKindIdCall] = BlockKindNew((BlockKind){
        .pinKind = PinKindExpression,
        .searchText = "call",
        .text = "call",
        .isGrowable = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindExpression),
                NewChildPin(PinKindExpression),
            },
        .defaultChildrenCount = 2,
    });
    BlockKinds[BlockKindIdIdentifier] = BlockKindNew((BlockKind){
        .pinKind = PinKindIdentifier,
        .defaultChildrenCount = 0,
    });
}

void BlockKindsDeinit(void)
{
    for (int32_t i = 0; i < BlockKindIdCount; i++)
    {
        free(BlockKinds[i].defaultChildren);
    }
}

void BlockKindsUpdateTextSize(Font *font)
{
    for (int32_t i = 0; i < BlockKindIdCount; i++)
    {
        if (BlockKinds[i].text)
        {
            GetTextSize(BlockKinds[i].text, &BlockKinds[i].textWidth, &BlockKinds[i].textHeight, font);
        }
    }
}

// TODO: NO GLOBALS!!!!
List_Block blockPool;
uint64_t *blockPoolUsedBitMasks;
uint64_t blockPoolUsedBitMasksCount;

int32_t BlockNew(BlockKindId kindId, int32_t parentI, int32_t childI)
{
    uint64_t unusedBlockI = 0;
    bool foundUnusedBlock = false;

    int32_t allocatedMasks = (int32_t)(blockPool.length / 64);
    for (int32_t maskI = 0; maskI < allocatedMasks; maskI++)
    {
        uint64_t mask = blockPoolUsedBitMasks[maskI];

        if (mask == 0xffffffffffffffff)
        {
            continue;
        }

        int32_t allocatedBits = 64;

        if (maskI == allocatedMasks - 1)
        {
            allocatedBits = blockPool.length % 64;
        }

        for (int32_t bitI = 0; bitI < allocatedBits; bitI++)
        {
            uint64_t isUsed = (mask >> bitI) & 1;

            if (!isUsed)
            {
                unusedBlockI = maskI * 64 + bitI;
                foundUnusedBlock = true;
                break;
            }
        }

        if (foundUnusedBlock)
        {
            break;
        }
    }

    int32_t blockI;

    if (foundUnusedBlock)
    {
        // printf("unused: %llu\n", unusedBlockI);
        blockPool.data[unusedBlockI] = (Block){0};
        blockI = (int32_t)unusedBlockI;
    }
    else
    {
        ListPush_Block(&blockPool, (Block){0});
        blockI = (int32_t)blockPool.length - 1;
        // printf("new: %d, %zu\n", blockI, blockPool.length);

        if (blockPoolUsedBitMasksCount < blockPool.capacity / 64)
        {
            blockPoolUsedBitMasksCount = blockPool.capacity / 64;
            blockPoolUsedBitMasks = realloc(blockPoolUsedBitMasks, blockPoolUsedBitMasksCount * sizeof(uint64_t));
            assert(blockPoolUsedBitMasks);
        }
    }

    blockPoolUsedBitMasks[blockI / 64] |= (uint64_t)1 << (blockI % 64);

    const BlockKind *kind = &BlockKinds[kindId];

    // Block *block = malloc(sizeof(Block));
    // assert(block);

    Block *block = &blockPool.data[blockI];

    *block = (Block){
        .kindId = kindId,
        .parentI = parentI,
    };

    if (kindId == BlockKindIdIdentifier)
    {
        return blockI;
    }

    block->data.parent = (BlockParentData){
        .childrenCapacity = kind->defaultChildrenCount,
        .childrenCount = kind->defaultChildrenCount,
    };

    if (kind->defaultChildrenCount > 0)
    {
        blockPool.data[blockI].data.parent.children = malloc(sizeof(int32_t) * kind->defaultChildrenCount);
        assert(blockPool.data[blockI].data.parent.children);

        for (int32_t i = 0; i < kind->defaultChildrenCount; i++)
        {
            if (kind->defaultChildren[i].isPin)
            {
                blockPool.data[blockI].data.parent.children[i] = BlockNew(BlockKindIdPin, blockI, i);
            }
            else
            {
                blockPool.data[blockI].data.parent.children[i] = BlockNew(kind->defaultChildren[i].blockKindId, blockI, i);
            }
        }
    }

    return blockI;
}

void BlockDelete(int32_t blockI)
{
    Block *block = &blockPool.data[blockI];

    if (block->kindId == BlockKindIdIdentifier)
    {
        free(block->data.identifier.text);
    }
    else
    {
        for (int32_t i = 0; i < block->data.parent.childrenCount; i++)
        {
            BlockDelete(block->data.parent.children[i]);
        }

        free(block->data.parent.children);
    }

    // free(block);
    blockPoolUsedBitMasks[blockI / 64] &= ~((uint64_t)1 << (blockI % 64));
}

int32_t BlockGetChildrenCount(int32_t blockI)
{
    Block *block = &blockPool.data[blockI];

    if (block->kindId == BlockKindIdIdentifier)
    {
        return 0;
    }

    return block->data.parent.childrenCount;
}

char *BlockGetText(int32_t blockI)
{
    Block *block = &blockPool.data[blockI];

    if (block->kindId == BlockKindIdIdentifier)
    {
        return block->data.identifier.text;
    }

    return BlockKinds[block->kindId].text;
}

void BlockGetTextSize(int32_t blockI, int32_t *width, int32_t *height)
{
    Block *block = &blockPool.data[blockI];

    if (block->kindId == BlockKindIdIdentifier)
    {
        *width = block->data.identifier.textWidth;
        *height = block->data.identifier.textHeight;
        return;
    }

    *width = BlockKinds[block->kindId].textWidth;
    *height = BlockKinds[block->kindId].textHeight;
}

// Frees the old child if it exists, expands this block's children array as necessary.
void BlockReplaceChild(int32_t blockI, int32_t childI, int32_t i)
{
    Block *block = &blockPool.data[blockI];
    assert(block->kindId != BlockKindIdIdentifier);

    BlockParentData *parentData = &block->data.parent;

    if (i >= parentData->childrenCount)
    {
        if (i >= parentData->childrenCapacity)
        {
            parentData->childrenCapacity *= 2;
            parentData->children = realloc(parentData->children, sizeof(int32_t) * parentData->childrenCapacity);
            assert(parentData->children);
        }

        i = parentData->childrenCount;
        parentData->childrenCount++;
    }
    else
    {
        BlockDelete(parentData->children[i]);
    }

    parentData->children[i] = childI;

    Block *child = &blockPool.data[childI];
    child->parentI = blockI;
}

uint64_t BlockCountAll(int32_t blockI)
{
    Block *block = &blockPool.data[blockI];

    uint64_t count = 1;

    for (int32_t i = 0; i < BlockGetChildrenCount(blockI); i++)
    {
        count += BlockCountAll(block->data.parent.children[i]);
    }

    return count;
}

static const int32_t BlockPadding = 6;
static const int32_t LineWidth = 3;

void BlockUpdateTree(int32_t blockI, int32_t x, int32_t y)
{
    Block *block = &blockPool.data[blockI];

    block->x = x;
    block->y = y;

    int32_t textWidth, textHeight;
    BlockGetTextSize(blockI, &textWidth, &textHeight);

    int32_t childrenCount = BlockGetChildrenCount(blockI);

    x += BlockPadding;

    if (childrenCount > 0)
    {
        y += BlockPadding;
    }
    else
    {
        y += textHeight;
    }

    BlockKind *kind = &BlockKinds[block->kindId];

    if (!kind->isTextInfix)
    {
        x += textWidth + BlockPadding;
    }

    int32_t startX = x;
    int32_t maxWidth = 0;

    if (kind->isVertical)
    {
        for (int32_t i = 0; i < childrenCount; i++)
        {
            int32_t childI = block->data.parent.children[i];
            Block *child = &blockPool.data[childI];

            BlockUpdateTree(childI, x, y);

            x += child->width + BlockPadding;
            y += child->height + BlockPadding;

            maxWidth = Int32Max(maxWidth, x - block->x);
            x = startX;
        }
    }
    else
    {
        int32_t maxHeight = 0;

        for (int32_t i = 0; i < childrenCount; i++)
        {
            int32_t childI = block->data.parent.children[i];
            Block *child = &blockPool.data[childI];

            BlockUpdateTree(childI, x, y);

            x += child->width + BlockPadding;

            if (i < childrenCount - 1 && kind->isTextInfix)
            {
                x += textWidth + BlockPadding;
            }

            maxHeight = Int32Max(maxHeight, child->height + BlockPadding);
        }

        maxWidth = Int32Max(maxWidth, x - block->x);
        x = startX;
        y += maxHeight;
    }

    block->width = maxWidth;
    block->height = y - block->y;
}

static Color BlockGetDepthColor(int32_t depth, Theme *theme)
{
    if (depth % 2 == 0)
    {
        return theme->evenColor;
    }
    else
    {
        return theme->oddColor;
    }
}

static void DrawRect(int32_t x, int32_t y, int32_t width, int32_t height)
{
    // TODO: Soon this should use floats to allow quality scaling.
    sgp_draw_filled_rect((float)x, (float)y, (float)width, (float)height);
}

static int32_t BlockFindFirstVisibleChildI(Block *block, int32_t childrenCount, int32_t minY)
{
    int32_t minI = 0;
    int32_t maxI = childrenCount - 1;

    if (maxI < 1)
    {
        return 0;
    }

    while (minI != maxI)
    {
        int32_t i = (minI + maxI) / 2;
        int32_t childI = block->data.parent.children[i];
        Block *child = &blockPool.data[childI];

        if (child->y + child->height < minY)
        {
            minI += 1;
        }
        else
        {
            maxI = i;
        }
    }

    return minI;
}

void BlockDraw(int32_t blockI, int32_t cursorBlockI, int32_t depth, int32_t minY, int32_t maxY, Font *font, Theme *theme)
{
    Block *block = &blockPool.data[blockI];

    if (blockI == cursorBlockI)
    {
        ColorSet(theme->cursorColor);

        DrawRect(block->x - BlockPadding - LineWidth, block->y - BlockPadding - LineWidth, block->width + LineWidth * 2,
            block->height + LineWidth * 2);
    }

    if (block->kindId == BlockKindIdPin)
    {
        ColorSet(theme->pinColor);
    }
    else
    {
        ColorSet(BlockGetDepthColor(depth, theme));
    }

    DrawRect(block->x - BlockPadding, block->y - BlockPadding, block->width, block->height);

    ColorSet(theme->textColor);

    int32_t textY = block->y - BlockPadding / 2;
    int32_t childrenCount = BlockGetChildrenCount(blockI);
    bool hasChildren = childrenCount > 0;

    if (!hasChildren)
    {
        textY -= BlockPadding;
    }

    BlockKind *kind = &BlockKinds[block->kindId];
    char *text = BlockGetText(blockI);

    // TODO: Also do this is the text is infix, but
    // the node has < 2 children. eg. so that -/+
    // can be used as unary operators too.
    // Allows writing -x, +x instead of 0-x, 0+x.
    if (!kind->isTextInfix)
    {
        DrawText(text, block->x, textY, font);
    }

    if (!hasChildren)
    {
        return;
    }

    int32_t firstVisibleI = BlockFindFirstVisibleChildI(block, childrenCount, minY);

    for (int32_t i = firstVisibleI; i < childrenCount; i++)
    {
        int32_t childI = block->data.parent.children[i];
        Block *child = &blockPool.data[childI];

        if (child->y > maxY)
        {
            break;
        }

        BlockDraw(childI, cursorBlockI, depth + 1, minY, maxY, font, theme);

        if (i < childrenCount - 1 && kind->isTextInfix)
        {
            DrawText(text, block->x + block->width, textY, font);
        }
    }
}