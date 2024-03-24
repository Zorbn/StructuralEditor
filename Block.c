#include "Block.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <stdio.h>
#include <string.h>

const BlockKind BlockKinds[] = {
    [BlockKindIdPin] = {
        .pinKind = PinKindNone,
        .text = ".",
    },
    [BlockKindIdDo] = {
        .pinKind = PinKindStatement,
        .searchText = "do",
        .text = "do",
        .isVertical = true,
        .isGrowable = true,
        .defaultChildren = (DefaultChildKind[]){
            {
                .isPin = true,
                .pinKind = PinKindStatement,
            },
        },
        .defaultChildrenCount = 1,
    },
    [BlockKindIdStatementList] = {
        .pinKind = PinKindNone,
        .text = "",
        .isVertical = true,
        .isGrowable = true,
        .defaultChildren = (DefaultChildKind[]){
            {
                .isPin = true,
                .pinKind = PinKindStatement,
            },
        },
        .defaultChildrenCount = 1,
    },
    [BlockKindIdFunctionHeader] = {
        .pinKind = PinKindNone,
        .text = "fn",
        .isGrowable = true,
        .defaultChildren = (DefaultChildKind[]){
            {
                .isPin = true,
                .pinKind = PinKindIdentifier,
            },
            {
                .isPin = true,
                .pinKind = PinKindIdentifier,
            },
        },
        .defaultChildrenCount = 2,
    },
    [BlockKindIdFunction] = {
        .pinKind = PinKindStatement,
        .searchText = "fn",
        .isVertical = true,
        .defaultChildren = (DefaultChildKind[]){
            {
                .blockKindId = BlockKindIdFunctionHeader,
            },
            {
                .blockKindId = BlockKindIdStatementList,
            },
        },
        .defaultChildrenCount = 2,
    },
    [BlockKindIdLambdaFunctionHeader] = {
        .pinKind = PinKindNone,
        .text = "fn",
        .isGrowable = true,
        .defaultChildren = (DefaultChildKind[]){
            {
                .isPin = true,
                .pinKind = PinKindIdentifier,
            },
        },
        .defaultChildrenCount = 1,
    },
    [BlockKindIdLambdaFunction] = {
        .pinKind = PinKindExpression,
        .searchText = "fn",
        .isVertical = true,
        .defaultChildren = (DefaultChildKind[]){
            {
                .blockKindId = BlockKindIdLambdaFunctionHeader,
            },
            {
                .blockKindId = BlockKindIdStatementList,
            },
        },
        .defaultChildrenCount = 2,
    },
    [BlockKindIdCase] = {
        .pinKind = PinKindNone,
        .text = "case",
        .isVertical = true,
        .isGrowable = true,
        .defaultChildren = (DefaultChildKind[]){
            {
                .isPin = true,
                .pinKind = PinKindExpression,
            },
            {
                .isPin = true,
                .pinKind = PinKindStatement,
            },
        },
        .defaultChildrenCount = 2,
    },
    [BlockKindIdIfCases] = {
        .pinKind = PinKindNone,
        .text = "if",
        .isVertical = true,
        .isGrowable = true,
        .defaultChildren = (DefaultChildKind[]){
            {
                .blockKindId = BlockKindIdCase,
            },
        },
        .defaultChildrenCount = 1,
    },
    [BlockKindIdElseCase] = {
        .pinKind = PinKindNone,
        .text = "else",
        .isVertical = true,
        .isGrowable = true,
        .defaultChildren = (DefaultChildKind[]){
            {
                .isPin = true,
                .pinKind = PinKindStatement,
            },
        },
        .defaultChildrenCount = 1,
    },
    [BlockKindIdIf] = {
        .pinKind = PinKindStatement,
        .searchText = "if",
        .isVertical = true,
        .defaultChildren = (DefaultChildKind[]){
            {
                .blockKindId = BlockKindIdIfCases,
            },
            {
                .blockKindId = BlockKindIdElseCase,
            },
        },
        .defaultChildrenCount = 2,
    },
    [BlockKindIdAssignment] = {
        .pinKind = PinKindStatement,
        .searchText = "=",
        .text = "=",
        .isTextInfix = true,
        .defaultChildren = (DefaultChildKind[]){
            {
                .isPin = true,
                .pinKind = PinKindExpression,
            },
            {
                .isPin = true,
                .pinKind = PinKindExpression,
            },
        },
        .defaultChildrenCount = 2,
    },
    [BlockKindIdAdd] = {
        .pinKind = PinKindExpression,
        .searchText = "+",
        .text = "+",
        .isTextInfix = true,
        .isGrowable = true,
        .defaultChildren = (DefaultChildKind[]){
            {
                .isPin = true,
                .pinKind = PinKindExpression,
            },
            {
                .isPin = true,
                .pinKind = PinKindExpression,
            },
            {
                .isPin = true,
                .pinKind = PinKindExpression,
            },
        },
        .defaultChildrenCount = 3,
    },
    [BlockKindIdCall] = {
        .pinKind = PinKindExpression,
        .searchText = "call",
        .text = "call",
        .isGrowable = true,
        .defaultChildren = (DefaultChildKind[]){
            {
                .isPin = true,
                .pinKind = PinKindExpression,
            },
            {
                .isPin = true,
                .pinKind = PinKindExpression,
            },
        },
        .defaultChildrenCount = 2,
    },
    [BlockKindIdIdentifier] = {
        .pinKind = PinKindIdentifier,
        .text = "",
        .defaultChildrenCount = 0,
    },
};

Block *BlockNew(BlockKindId kindId, Block *parent)
{
    Block *block = malloc(sizeof(Block));
    *block = (Block){
        .kindId = kindId,
        .parent = parent,
        .children = NULL,
    };

    const BlockKind *kind = &BlockKinds[kindId];
    if (kind->defaultChildrenCount > 0)
    {
        block->children = malloc(sizeof(Block *) * kind->defaultChildrenCount);
        block->childrenCapacity = kind->defaultChildrenCount;

        for (int64_t i = 0; i < kind->defaultChildrenCount; i++)
        {
            block->children[i] = BlockNew(kind->defaultChildren[i].blockKindId, block);
        }

        block->childrenCount = kind->defaultChildrenCount;
    }

    return block;
}

void BlockDelete(Block *block)
{
    for (int64_t i = 0; i < block->childrenCount; i++)
    {
        BlockDelete(block->children[i]);
    }

    free(block->children);
    free(block->text);
    free(block);
}

// Frees the old child if it exists, expands this block's children array as necessary.
void BlockReplaceChild(Block *block, Block *child, int64_t i)
{
    if (i >= block->childrenCount)
    {
        if (i >= block->childrenCapacity)
        {
            block->childrenCapacity *= 2;
            block->children = realloc(block->children, sizeof(Block *) * block->childrenCapacity);
        }

        i = block->childrenCount;
        block->childrenCount++;
    }
    else
    {
        BlockDelete(block->children[i]);
    }

    block->children[i] = child;
}

uint64_t BlockCountAll(Block *block)
{
    uint64_t count = 1;

     for (int64_t i = 0; i < block->childrenCount; i++)
    {
        count += BlockCountAll(block->children[i]);
    }

    return count;
}

uint64_t BlockSizeAll(Block *block)
{
    uint64_t size = sizeof(Block) + sizeof(Block*) * block->childrenCapacity;

    if (block->text)
    {
        size += strlen(block->text) + 1;
    }

    for (int64_t i = 0; i < block->childrenCount; i++)
    {
        size += BlockSizeAll(block->children[i]);
    }

    return size;
}