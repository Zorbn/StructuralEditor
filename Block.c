#include "Block.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>

#define SOKOL_GLCORE33
#include <sokol_gfx.h>
#include <sokol_gp.h>
#include <sokol_log.h>

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

BlockKind BlockKindNew(BlockKind blockKind, Font *font)
{
    DefaultChildKind *defaultChildren = malloc(sizeof(DefaultChildKind) * blockKind.defaultChildrenCount);

    for (int32_t i = 0; i < blockKind.defaultChildrenCount; i++)
    {
        defaultChildren[i] = blockKind.defaultChildren[i];
    }

    blockKind.defaultChildren = defaultChildren;

    if (blockKind.text)
    {
        GetTextSize(blockKind.text, &blockKind.textWidth, &blockKind.textHeight, font);
    }

    return blockKind;
}

BlockKind BlockKinds[BlockKindIdCount];

void BlockKindsInit(Font *font)
{
    BlockKinds[BlockKindIdPin] = BlockKindNew(
        (BlockKind){
            .pinKind = PinKindNone,
            .text = ".",
        },
        font);
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
        },
        font);
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
        },
        font);
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
        },
        font);
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
        },
        font);
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
        },
        font);
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
        },
        font);
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
        },
        font);
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
        },
        font);
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
        },
        font);
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
        },
        font);
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
        },
        font);
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
        },
        font);
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
        },
        font);
    BlockKinds[BlockKindIdIdentifier] = BlockKindNew(
        (BlockKind){
            .pinKind = PinKindIdentifier,
            .defaultChildrenCount = 0,
        },
        font);
}

void BlockKindsDeinit(void)
{
    for (int32_t i = 0; i < BlockKindIdCount; i++)
    {
        free(BlockKinds[i].defaultChildren);
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
        .childrenCapacity = kind->defaultChildrenCount,
        .childrenCount = kind->defaultChildrenCount,
    };

    if (kind->defaultChildrenCount > 0)
    {
        block->children = malloc(sizeof(Block *) * kind->defaultChildrenCount);
        assert(block->children);

        for (int32_t i = 0; i < kind->defaultChildrenCount; i++)
        {
            if (kind->defaultChildren[i].isPin)
            {
                block->children[i] = BlockNew(BlockKindIdPin, block, i);
            }
            else
            {
                block->children[i] = BlockNew(kind->defaultChildren[i].blockKindId, block, i);
            }
        }
    }

    return block;
}

void BlockDelete(Block *block)
{
    for (int32_t i = 0; i < block->childrenCount; i++)
    {
        BlockDelete(block->children[i]);
    }

    free(block->text);
    free(block->children);
    free(block);
}

// Frees the old child if it exists, expands this block's children array as necessary.
void BlockReplaceChild(Block *block, Block *child, int32_t i)
{
    if (i >= block->childrenCount)
    {
        if (i >= block->childrenCapacity)
        {
            block->childrenCapacity *= 2;
            block->children = realloc(block->children, sizeof(Block *) * block->childrenCapacity);
            assert(block->children);
        }

        i = block->childrenCount;
        block->childrenCount++;
    }
    else
    {
        BlockDelete(block->children[i]);
    }

    block->children[i] = child;
    child->parent = block;
    child->childI = i;
}

uint64_t BlockCountAll(Block *block)
{
    uint64_t count = 1;

    for (int32_t i = 0; i < block->childrenCount; i++)
    {
        count += BlockCountAll(block->children[i]);
    }

    return count;
}

uint64_t BlockSizeAll(Block *block)
{
    uint64_t size = sizeof(Block) + sizeof(Block *) * block->childrenCapacity;

    if (block->text)
    {
        size += strlen(block->text) + 1;
    }

    for (int32_t i = 0; i < block->childrenCount; i++)
    {
        size += BlockSizeAll(block->children[i]);
    }

    return size;
}

const int32_t BlockPadding = 6;
const int32_t LineWidth = 3;

DrawResult BlockDrawOne(Block *block, int32_t childrenWidth, int32_t childrenHeight, Font *font)
{
    const BlockKind *kind = &BlockKinds[block->kindId];

    int32_t textWidth = kind->textWidth;
    int32_t textHeight = kind->textHeight;
    char *text = kind->text;

    if (block->text)
    {
        text = block->text;
        GetTextSize(block->text, &textWidth, &textHeight, font);
    }

    int32_t width = childrenWidth + textWidth + BlockPadding * 2;
    int32_t height = childrenHeight + textHeight + BlockPadding * 2;

    return (DrawResult){
        .x = textWidth,
        .y = textHeight,
        .width = width,
        .height = height,
    };
}

// Starting from this block, draw until reaching the edge of the screen.
DrawResult BlockDraw(Block *block, int32_t x, int32_t y, int32_t minY, int32_t maxY, Font *font, List_DrawCommand *drawCommands)
{
    if (y > maxY)
    {
        return (DrawResult){0};
    }

    int32_t width = 0;
    int32_t height = 0;
    // int32_t startX = x;
    for (int32_t i = 0; i < block->childrenCount; i++)
    {
        if (y + height > maxY)
        {
            break;
        }

        DrawResult result = BlockDraw(block->children[i], x, y + height, minY, maxY, font, drawCommands);
//
//         if (result.x < x)
        {
            x = result.x;
        }

        if (result.width > width)
        {
            width = result.width;
        }

        height += result.height;
    }

    DrawResult result = BlockDrawOne(block, width, height, font);
    result.x = x - result.x;
    // result.y = y - result.y;

    DrawCommand command = (DrawCommand){
        .block = block,
        .x = result.x,
        .y = y,
        .width = result.width,
        .height = result.height,
    };
    ListPush_DrawCommand(drawCommands, command);

    return result;
}

void DrawCommandHandle(List_DrawCommand *drawCommands, Font *font)
{
    for (int32_t i = (int32_t)drawCommands->length - 1; i >= 0; i--)
    {
        const DrawCommand *command = &drawCommands->data[i];
        const BlockKind *kind = &BlockKinds[command->block->kindId];

        // printf("%d, %d, %d, %d\n", command->x, command->y, command->width, command->height);

        char *text = kind->text;

        if (command->block->text)
        {
            text = command->block->text;
        }

        sgp_set_color(1.0f, 1.0f, 1.0f, 1.0f);
        sgp_draw_filled_rect((float)command->x, (float)command->y, (float)command->width, (float)command->height);
        sgp_set_color(0.0f, 0.0f, 0.0f, 1.0f);
        sgp_draw_filled_rect((float)command->x + LineWidth, (float)command->y + LineWidth,
            (float)command->width - LineWidth * 2, (float)command->height - LineWidth * 2);

        if (text)
        {
            float textY = (float)(command->y + BlockPadding) - BlockPadding * 0.5f;
            sgp_set_color(1.0f, 1.0f, 1.0f, 1.0f);
            DrawText(text, (float)(command->x + BlockPadding), textY, font);
        }
    }

    // printf("\n\n\n\n");
}