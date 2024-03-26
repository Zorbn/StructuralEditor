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
    BlockKinds[BlockKindIdPin] = BlockKindNew(
        (BlockKind){
            .pinKind = PinKindNone,
            .text = ".",
        });
    BlockKinds[BlockKindIdDo] = BlockKindNew(
        (BlockKind){
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
    BlockKinds[BlockKindIdStatementList] = BlockKindNew(
        (BlockKind){
            .pinKind = PinKindNone,
            .isVertical = true,
            .isGrowable = true,
            .defaultChildren =
                (DefaultChildKind[]){
                    NewChildPin(PinKindStatement),
                },
            .defaultChildrenCount = 1,
        });
    BlockKinds[BlockKindIdFunctionHeader] = BlockKindNew(
        (BlockKind){
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
    BlockKinds[BlockKindIdFunction] = BlockKindNew(
        (BlockKind){
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
    BlockKinds[BlockKindIdLambdaFunctionHeader] = BlockKindNew(
        (BlockKind){
            .pinKind = PinKindNone,
            .text = "fn",
            .isGrowable = true,
            .defaultChildren =
                (DefaultChildKind[]){
                    NewChildPin(PinKindIdentifier),
                },
            .defaultChildrenCount = 1,
        });
    BlockKinds[BlockKindIdLambdaFunction] = BlockKindNew(
        (BlockKind){
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
    BlockKinds[BlockKindIdCase] = BlockKindNew(
        (BlockKind){
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
    BlockKinds[BlockKindIdIfCases] = BlockKindNew(
        (BlockKind){
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
    BlockKinds[BlockKindIdElseCase] = BlockKindNew(
        (BlockKind){
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
    BlockKinds[BlockKindIdIf] = BlockKindNew(
        (BlockKind){
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
    BlockKinds[BlockKindIdAssignment] = BlockKindNew(
        (BlockKind){
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
    BlockKinds[BlockKindIdAdd] = BlockKindNew(
        (BlockKind){
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
    BlockKinds[BlockKindIdCall] = BlockKindNew(
        (BlockKind){
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
    BlockKinds[BlockKindIdIdentifier] = BlockKindNew(
        (BlockKind){
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
    };

    if (kindId == BlockKindIdIdentifier)
    {
        return block;
    }

    block->data.parent = (BlockParentData){
        .childrenCapacity = kind->defaultChildrenCount,
        .childrenCount = kind->defaultChildrenCount,
    };

    if (kind->defaultChildrenCount > 0)
    {
        block->data.parent.children = malloc(sizeof(Block *) * kind->defaultChildrenCount);
        assert(block->children);

        for (int32_t i = 0; i < kind->defaultChildrenCount; i++)
        {
            if (kind->defaultChildren[i].isPin)
            {
                block->data.parent.children[i] = BlockNew(BlockKindIdPin, block, i);
            }
            else
            {
                block->data.parent.children[i] = BlockNew(kind->defaultChildren[i].blockKindId, block, i);
            }
        }
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
        for (int32_t i = 0; i < block->data.parent.childrenCount; i++)
        {
            BlockDelete(block->data.parent.children[i]);
        }

        free(block->data.parent.children);
    }

    free(block);
}

int32_t BlockGetChildrenCount(Block *block)
{
    if (block->kindId == BlockKindIdIdentifier)
    {
        return 0;
    }

    return block->data.parent.childrenCount;
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

// Frees the old child if it exists, expands this block's children array as necessary.
void BlockReplaceChild(Block *block, Block *child, int32_t i)
{
    assert(block->kindId != BlockKindIdIdentifier);

    BlockParentData *parentData = &block->data.parent;

    if (i >= parentData->childrenCount)
    {
        if (i >= parentData->childrenCapacity)
        {
            parentData->childrenCapacity *= 2;
            parentData->children = realloc(parentData->children, sizeof(Block *) * parentData->childrenCapacity);
            assert(block->children);
        }

        i = parentData->childrenCount;
        parentData->childrenCount++;
    }
    else
    {
        BlockDelete(parentData->children[i]);
    }

    parentData->children[i] = child;
    child->parent = block;
}

uint64_t BlockCountAll(Block *block)
{
    uint64_t count = 1;

    for (int32_t i = 0; i < BlockGetChildrenCount(block); i++)
    {
        count += BlockCountAll(block->data.parent.children[i]);
    }

    return count;
}

static const int32_t BlockPadding = 6;
static const int32_t LineWidth = 3;

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
            Block *child = block->data.parent.children[i];

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
            Block *child = block->data.parent.children[i];

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

void BlockDraw(Block *block, Block *cursorBlock, int32_t depth, int32_t minY, int32_t maxY, Font *font, Theme *theme)
{
    // printf("%d, %d, %d, %d\n", block->x, block->y, block->width, block->height);

    if (block == cursorBlock)
    {
        ColorSet(theme->cursorColor);

        DrawRect(block->x - BlockPadding - LineWidth, block->y - BlockPadding - LineWidth,
            block->width + LineWidth * 2, block->height + LineWidth * 2);
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

    BlockKind *kind = &BlockKinds[block->kindId];
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

    // TODO: Only render visible children.
    for (int32_t i = 0; i < childrenCount; i++)
    {
        Block *child = block->data.parent.children[i];

        BlockDraw(child, cursorBlock, depth + 1, minY, maxY, font, theme);

        if (i < childrenCount - 1 && kind->isTextInfix)
        {
            DrawText(text, block->x + block->width, textY, font);
        }
    }
}

// DrawResult BlockDrawOne(Block *block, int32_t childrenWidth, int32_t childrenHeight, Font *font)
// {
//     const BlockKind *kind = &BlockKinds[block->kindId];

//     int32_t textWidth = kind->textWidth;
//     int32_t textHeight = kind->textHeight;
//     char *text = kind->text;

//     if (block->text)
//     {
//         text = block->text;
//         GetTextSize(block->text, &textWidth, &textHeight, font);
//     }

//     int32_t width = childrenWidth + textWidth + BlockPadding * 2;
//     int32_t height = childrenHeight + textHeight + BlockPadding * 2;

//     return (DrawResult){
//         .x = textWidth,
//         .y = textHeight,
//         .width = width,
//         .height = height,
//     };
// }

// // Starting from this block, draw until reaching the edge of the screen.
// DrawResult BlockDraw(Block *block, int32_t x, int32_t y, int32_t minY, int32_t maxY, Font *font, List_DrawCommand *drawCommands)
// {
//     if (y > maxY)
//     {
//         return (DrawResult){0};
//     }

//     int32_t width = 0;
//     int32_t height = 0;
//     // int32_t startX = x;
//     for (int32_t i = 0; i < block->childrenCount; i++)
//     {
//         if (y + height > maxY)
//         {
//             break;
//         }

//         DrawResult result = BlockDraw(block->children[i], x, y + height, minY, maxY, font, drawCommands);
// //
// //         if (result.x < x)
//         {
//             x = result.x;
//         }

//         if (result.width > width)
//         {
//             width = result.width;
//         }

//         height += result.height;
//     }

//     DrawResult result = BlockDrawOne(block, width, height, font);
//     result.x = x - result.x;
//     // result.y = y - result.y;

//     DrawCommand command = (DrawCommand){
//         .block = block,
//         .x = result.x,
//         .y = y,
//         .width = result.width,
//         .height = result.height,
//     };
//     ListPush_DrawCommand(drawCommands, command);

//     return result;
// }

// void DrawCommandHandle(List_DrawCommand *drawCommands, Font *font)
// {
//     for (int32_t i = (int32_t)drawCommands->length - 1; i >= 0; i--)
//     {
//         const DrawCommand *command = &drawCommands->data[i];
//         const BlockKind *kind = &BlockKinds[command->block->kindId];

//         // printf("%d, %d, %d, %d\n", command->x, command->y, command->width, command->height);

//         char *text = kind->text;

//         if (command->block->text)
//         {
//             text = command->block->text;
//         }

//         sgp_set_color(1.0f, 1.0f, 1.0f, 1.0f);
//         sgp_draw_filled_rect((float)command->x, (float)command->y, (float)command->width, (float)command->height);
//         sgp_set_color(0.0f, 0.0f, 0.0f, 1.0f);
//         sgp_draw_filled_rect((float)command->x + LineWidth, (float)command->y + LineWidth,
//             (float)command->width - LineWidth * 2, (float)command->height - LineWidth * 2);

//         if (text)
//         {
//             float textY = (float)(command->y + BlockPadding) - BlockPadding * 0.5f;
//             sgp_set_color(1.0f, 1.0f, 1.0f, 1.0f);
//             DrawText(text, (float)(command->x + BlockPadding), textY, font);
//         }
//     }

//     // printf("\n\n\n\n");
// }