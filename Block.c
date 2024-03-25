#include "Block.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <stdio.h>
#include <string.h>
#include <assert.h>

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
        .defaultChildren = (DefaultChildKind[]){
            NewChildPin(PinKindStatement),
        },
        .defaultChildrenCount = 1,
    });
    BlockKinds[BlockKindIdStatementList] = BlockKindNew((BlockKind){
        .pinKind = PinKindNone,
        .text = "",
        .isVertical = true,
        .isGrowable = true,
        .defaultChildren = (DefaultChildKind[]){
            NewChildPin(PinKindStatement),
        },
        .defaultChildrenCount = 1,
    });
    BlockKinds[BlockKindIdFunctionHeader] = BlockKindNew((BlockKind){
        .pinKind = PinKindNone,
        .text = "fn",
        .isGrowable = true,
        .defaultChildren = (DefaultChildKind[]){
            NewChildPin(PinKindIdentifier),
            NewChildPin(PinKindIdentifier),
        },
        .defaultChildrenCount = 2,
    });
    BlockKinds[BlockKindIdFunction] = BlockKindNew((BlockKind){
        .pinKind = PinKindStatement,
        .searchText = "fn",
        .isVertical = true,
        .defaultChildren = (DefaultChildKind[]){
            NewChild(BlockKindIdFunctionHeader),
            NewChild(BlockKindIdStatementList),
        },
        .defaultChildrenCount = 2,
    });
    BlockKinds[BlockKindIdLambdaFunctionHeader] = BlockKindNew((BlockKind){
        .pinKind = PinKindNone,
        .text = "fn",
        .isGrowable = true,
        .defaultChildren = (DefaultChildKind[]){
            NewChildPin(PinKindIdentifier),
        },
        .defaultChildrenCount = 1,
    });
    BlockKinds[BlockKindIdLambdaFunction] = BlockKindNew((BlockKind){
        .pinKind = PinKindExpression,
        .searchText = "fn",
        .isVertical = true,
        .defaultChildren = (DefaultChildKind[]){
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
        .defaultChildren = (DefaultChildKind[]){
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
        .defaultChildren = (DefaultChildKind[]){
            NewChild(BlockKindIdCase),
        },
        .defaultChildrenCount = 1,
    });
    BlockKinds[BlockKindIdElseCase] = BlockKindNew((BlockKind){
        .pinKind = PinKindNone,
        .text = "else",
        .isVertical = true,
        .isGrowable = true,
        .defaultChildren = (DefaultChildKind[]){
            NewChildPin(PinKindStatement),
        },
        .defaultChildrenCount = 1,
    });
    BlockKinds[BlockKindIdIf] = BlockKindNew((BlockKind){
        .pinKind = PinKindStatement,
        .searchText = "if",
        .isVertical = true,
        .defaultChildren = (DefaultChildKind[]){
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
        .defaultChildren = (DefaultChildKind[]){
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
        .defaultChildren = (DefaultChildKind[]){
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
        .defaultChildren = (DefaultChildKind[]){
            NewChildPin(PinKindExpression),
            NewChildPin(PinKindExpression),
        },
        .defaultChildrenCount = 2,
    });
    BlockKinds[BlockKindIdIdentifier] = BlockKindNew((BlockKind){
        .pinKind = PinKindIdentifier,
        .text = "",
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

Block *BlockNew(BlockKindId kindId, Block *parent)
{
    const BlockKind *kind = &BlockKinds[kindId];

    Block *block = malloc(sizeof(Block) + sizeof(Block *) * kind->defaultChildrenCount);
    assert(block);

    *block = (Block){
        .kindId = kindId,
        .parent = parent,
        .childrenCapacity = kind->defaultChildrenCount,
        .childrenCount = kind->defaultChildrenCount,
    };

    for (int64_t i = 0; i < kind->defaultChildrenCount; i++)
    {
        if (kind->defaultChildren[i].isPin)
        {
            block->children[i] = BlockNew(BlockKindIdPin, block);
        }
        else
        {
            block->children[i] = BlockNew(kind->defaultChildren[i].blockKindId, block);
        }
    }

    return block;
}

void BlockDelete(Block *block)
{
    for (int64_t i = 0; i < block->childrenCount; i++)
    {
        BlockDelete(block->children[i]);
    }

    free(block->text);
    free(block);
}

// Frees the old child if it exists, expands this block's children array as necessary.
void BlockReplaceChild(Block **block, Block *child, int64_t i)
{
    if (i >= (*block)->childrenCount)
    {
        if (i >= (*block)->childrenCapacity)
        {
            (*block)->childrenCapacity *= 2;
            *block = realloc(*block, sizeof(Block) + sizeof(Block *) * (*block)->childrenCapacity);
            assert(*block);
        }

        i = (*block)->childrenCount;
        (*block)->childrenCount++;
    }
    else
    {
        BlockDelete((*block)->children[i]);
    }

    (*block)->children[i] = child;
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
    uint64_t size = sizeof(Block) + sizeof(Block *) * block->childrenCapacity;

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