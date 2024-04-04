#include "Block.h"
#include "Math.h"
#include "Shapes.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>

static DefaultChildKind NewChild(BlockKindId childBlockKindId)
{
    return (DefaultChildKind){
        .blockKindId = childBlockKindId,
        .isPin = false,
    };
}

static DefaultChildKind NewChildPin(PinKind childPinKind)
{
    return (DefaultChildKind){
        .pinKind = childPinKind,
        .isPin = true,
    };
}

static PinKind DefaultChildKindGetPinKind(DefaultChildKind *defaultChildKind)
{
    if (defaultChildKind->isPin)
    {
        return defaultChildKind->pinKind;
    }

    return BlockKinds[defaultChildKind->blockKindId].pinKind;
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

const int32_t BlockPadding = 6;

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
        .text = "function",
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
        .text = "function",
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
            FontGetTextSize(BlockKinds[i].text, &BlockKinds[i].textWidth, &BlockKinds[i].textHeight, font);
        }
    }
}

// TODO: Have a way to create a block without creating it's children,
// this could be used in the parser when we know we're going to
// overwrite the new block's children anyway.
Block *BlockNew(BlockKindId kindId, Block *parent, int32_t childI)
{
    const BlockKind *kind = &BlockKinds[kindId];

    Block *block = malloc(sizeof(Block));
    assert(block);

    *block = (Block){
        .kindId = kindId,
        .parent = parent,
        .childI = childI,
        .y = INT32_MAX,
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

    FontGetTextSize(identifierData->text, &identifierData->textWidth, &identifierData->textHeight, font);

    return block;
}

Block *BlockCopy(Block *other, Block *parent, int32_t childI)
{
    Block *block = malloc(sizeof(Block));
    assert(block);

    *block = *other;
    block->parent = parent;
    block->childI = childI;

    if (other->kindId == BlockKindIdIdentifier)
    {
        int32_t textLength = (int32_t)strlen(other->data.identifier.text);
        block->data.identifier.text = calloc(textLength + 1, sizeof(char));
        memcpy(block->data.identifier.text, other->data.identifier.text, textLength);

        return block;
    }

    BlockParentData *parentData = &block->data.parent;
    BlockParentData *otherParentData = &other->data.parent;

    parentData->children = ListNew_BlockPointer(otherParentData->children.count);

    for (int32_t i = 0; i < otherParentData->children.count; i++)
    {
        ListPush_BlockPointer(&parentData->children, BlockCopy(otherParentData->children.data[i], block, i));
    }

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

static void BlockMarkChildrenNeedsUpdate(Block *block)
{
    block->y = INT32_MAX;

    int32_t childrenCount = BlockGetChildrenCount(block);

    for (int32_t i = 0; i < childrenCount; i++)
    {
        Block *child = block->data.parent.children.data[i];
        BlockMarkChildrenNeedsUpdate(child);
    }
}

void BlockMarkNeedsUpdate(Block *block)
{
    BlockMarkChildrenNeedsUpdate(block);

    Block *parent = block->parent;

    while (parent && parent->y != INT32_MAX)
    {
        parent->y = INT32_MAX;
        parent = parent->parent;
    }
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
    childI = MathInt32Min(childI, kind->defaultChildrenCount - 1);

    return &kind->defaultChildren[childI];
}

bool BlockCanSwapWith(Block *block, DefaultChildKind *otherDefaultChildKind)
{
    if (!block->parent)
    {
        return false;
    }

    DefaultChildKind *defaultChildKind = BlockGetDefaultChild(block->parent, block->childI);

    PinKind pinKind = DefaultChildKindGetPinKind(defaultChildKind);
    PinKind otherPinKind = DefaultChildKindGetPinKind(otherDefaultChildKind);

    return pinKind == otherPinKind;
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

void BlockInsertChild(Block *block, Block *child, int32_t childI)
{
    assert(block->kindId != BlockKindIdIdentifier);

    BlockParentData *parentData = &block->data.parent;

    for (int32_t i = childI; i < parentData->children.count; i++)
    {
        parentData->children.data[i]->childI += 1;
    }

    child->parent = block;
    child->childI = childI;
    ListInsert_BlockPointer(&parentData->children, child, childI);
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

void BlockUpdateTree(Block *block, int32_t x, int32_t y)
{
    if (block->y != INT32_MAX)
    {
        return;
    }

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

            maxWidth = MathInt32Max(maxWidth, x - block->x);
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

            maxHeight = MathInt32Max(maxHeight, child->height + BlockPadding);
        }

        maxWidth = MathInt32Max(maxWidth, x - block->x);
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

static int32_t BlockFindFirstVisibleChildI(Block *block, int32_t childrenCount, Camera *camera)
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

        if (child->y + child->height < camera->y)
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

void BlockDraw(Block *block, Block *cursorBlock, int32_t depth, Camera *camera, Font *font, Theme *theme)
{
    if (block->kindId == BlockKindIdPin)
    {
        ColorSet(theme->pinColor);
    }
    else
    {
        ColorSet(BlockGetDepthColor(depth, theme));
    }

    DrawRect((float)block->x - BlockPadding, (float)block->y - BlockPadding, (float)block->width, (float)block->height, camera->zoom);

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
        FontDraw(text, block->x * camera->zoom, textY * camera->zoom, font);
    }

    if (!hasChildren)
    {
        return;
    }

    int32_t firstVisibleI = BlockFindFirstVisibleChildI(block, childrenCount, camera);

    for (int32_t i = firstVisibleI; i < childrenCount; i++)
    {
        Block *child = block->data.parent.children.data[i];

        if (child->y > camera->y + camera->height / camera->zoom)
        {
            break;
        }

        BlockDraw(child, cursorBlock, depth + 1, camera, font, theme);

        if (i < childrenCount - 1 && kind->isTextInfix)
        {
            FontDraw(text, (child->x + child->width) * camera->zoom, textY * camera->zoom, font);
        }
    }
}