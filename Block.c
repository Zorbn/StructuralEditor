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

static const int32_t BlockPadding = 6;
static const int32_t LineWidth = 3;

const PinKindInsertBlocks PinInsertBlocks[] = {
    [PinKindExpression] = {
        .blockKindIds = (BlockKindId[]){
            BlockKindIdLambdaFunction,
            BlockKindIdAdd,
            BlockKindIdCall,
        },
        .blockKindIdCount = 3,
    },
    [PinKindStatement] = {
        .blockKindIds = (BlockKindId[]){
            BlockKindIdAssignment,
            BlockKindIdDo,
            BlockKindIdFunction,
            BlockKindIdIf,
            BlockKindIdCall,
        },
        .blockKindIdCount = 5,
    },
    [PinKindIdentifier] = {
        .blockKindIds = (BlockKindId[]){
            BlockKindIdIdentifier,
        },
        .blockKindIdCount = 1,
    },
    [PinKindNone] = {
        .blockKindIdCount = 0,
    },
};

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

Block *BlockNew(BlockKindId kindId, Block *parent, int32_t childI)
{
    const BlockKind *kind = &BlockKinds[kindId];

    Block *block = malloc(sizeof(Block));
    assert(block);

    *block = (Block){
        .kindId = kindId,
        .parent = parent,
        .childI = childI,
    };

    if (kindId == BlockKindIdIdentifier)
    {
        return block;
    }

    block->data.parent = (BlockParentData){
        .children = ListNew_BlockPointer(kind->defaultChildrenCount),
    };

    for (int32_t i = 0; i < kind->defaultChildrenCount; i++)
    {
        if (kind->defaultChildren[i].isPin)
        {
            ListPush_BlockPointer(&block->data.parent.children, BlockNew(BlockKindIdPin, block, i));
        }
        else
        {
            ListPush_BlockPointer(&block->data.parent.children, BlockNew(kind->defaultChildren[i].blockKindId, block, i));
        }
    }

    return block;
}

Block *BlockNewIdentifier(char *text, int32_t textCount, Font *font, Block *parent, int32_t childI)
{
    Block *block = BlockNew(BlockKindIdIdentifier, parent, childI);
    BlockIdentifierData *identifierData = &block->data.identifier;

    identifierData->text = malloc(textCount + 1);
    strncpy(identifierData->text, text, textCount);
    identifierData->text[textCount] = '\0';

    GetTextSize(identifierData->text, &identifierData->textWidth, &identifierData->textHeight, font);

    return block;
}

void BlockDelete(Block *block)
{
    if (block->kindId == BlockKindIdIdentifier)
    {
        free(block->data.identifier.text);
    }
    else
    {
        for (int32_t i = 0; i < block->data.parent.children.count; i++)
        {
            BlockDelete(block->data.parent.children.data[i]);
        }

        ListDelete_BlockPointer(&block->data.parent.children);
    }

    free(block);
}

int32_t BlockGetChildrenCount(Block *block)
{
    if (block->kindId == BlockKindIdIdentifier)
    {
        return 0;
    }

    return block->data.parent.children.count;
}

char *BlockGetText(Block *block)
{
    if (block->kindId == BlockKindIdIdentifier)
    {
        return block->data.identifier.text;
    }

    return BlockKinds[block->kindId].text;
}

void BlockGetTextSize(Block *block, int32_t *width, int32_t *height)
{
    if (block->kindId == BlockKindIdIdentifier)
    {
        *width = block->data.identifier.textWidth;
        *height = block->data.identifier.textHeight;
        return;
    }

    *width = BlockKinds[block->kindId].textWidth;
    *height = BlockKinds[block->kindId].textHeight;
}

DefaultChildKind *BlockGetDefaultChild(Block *block, int32_t childI)
{
    const BlockKind *kind = &BlockKinds[block->kindId];
    childI = Int32Min(childI, kind->defaultChildrenCount - 1);

    return &kind->defaultChildren[childI];
}

// Frees the old child if it exists, expands this block's children array as necessary.
void BlockReplaceChild(Block *block, Block *child, int32_t childI)
{
    assert(block->kindId != BlockKindIdIdentifier);

    BlockParentData *parentData = &block->data.parent;

    if (childI >= parentData->children.count)
    {
        childI = parentData->children.count;
        ListPush_BlockPointer(&parentData->children, (BlockPointer){0});
    }
    else
    {
        BlockDelete(parentData->children.data[childI]);
    }

    parentData->children.data[childI] = child;
    child->parent = block;
    child->childI = childI;
}

uint64_t BlockCountAll(Block *block)
{
    uint64_t count = 1;

    for (int32_t i = 0; i < BlockGetChildrenCount(block); i++)
    {
        count += BlockCountAll(block->data.parent.children.data[i]);
    }

    return count;
}

// TODO: Have a Tree struct that contains a reference to the root block, it should have a method TreeMarkDirty which will
// be used instead of calling BlockUpdateTree on the root node. Have a method TreeUpdate which is called once per frame,
// and calls BlockUpdateTree on the root node if it is dirty, the unmarks the dirty flag. This is to prevent multiple
// redundant calls to BlockUpdateTree per frame, when it really only needs to be called once before drawing if the tree
// was modified.
void BlockUpdateTree(Block *block, int32_t x, int32_t y)
{
    block->x = x;
    block->y = y;

    int32_t textWidth, textHeight;
    BlockGetTextSize(block, &textWidth, &textHeight);

    int32_t childrenCount = BlockGetChildrenCount(block);

    x += BlockPadding;

    if (childrenCount > 0)
    {
        y += BlockPadding;
    }
    else
    {
        y += textHeight;
    }

    const BlockKind *kind = &BlockKinds[block->kindId];

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
            Block *child = block->data.parent.children.data[i];

            BlockUpdateTree(child, x, y);

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
            Block *child = block->data.parent.children.data[i];

            BlockUpdateTree(child, x, y);

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

    return theme->oddColor;
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
        Block *child = block->data.parent.children.data[i];

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

void BlockDraw(Block *block, Block *cursorBlock, int32_t depth, int32_t minY, int32_t maxY, Font *font, Theme *theme)
{
    if (block == cursorBlock)
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
    int32_t childrenCount = BlockGetChildrenCount(block);
    bool hasChildren = childrenCount > 0;

    if (!hasChildren)
    {
        textY -= BlockPadding;
    }

    const BlockKind *kind = &BlockKinds[block->kindId];
    char *text = BlockGetText(block);

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
        Block *child = block->data.parent.children.data[i];

        if (child->y > maxY)
        {
            break;
        }

        BlockDraw(child, cursorBlock, depth + 1, minY, maxY, font, theme);

        if (i < childrenCount - 1 && kind->isTextInfix)
        {
            DrawText(text, block->x + block->width, textY, font);
        }
    }
}