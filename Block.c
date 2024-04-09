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

// static PinKind DefaultChildKindGetPinKind(DefaultChildKind *defaultChildKind)
// {
//     if (defaultChildKind->isPin)
//     {
//         return defaultChildKind->pinKind;
//     }

//     return BlockKinds[defaultChildKind->blockKindId].pinKind;
// }

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

const PinKindValidBlockSet PinKindValidBlocks[] = {
    [PinKindExpression] = {
        .blockKindIds = (BlockKindId[]){
            BlockKindIdLambdaFunction,
            BlockKindIdNot,
            BlockKindIdLength,
            BlockKindIdConcatenate,
            BlockKindIdModulo,
            BlockKindIdDivide,
            BlockKindIdMultiply,
            BlockKindIdSubtract,
            BlockKindIdAdd,
            BlockKindIdGreaterEqual,
            BlockKindIdLessEqual,
            BlockKindIdGreater,
            BlockKindIdLess,
            BlockKindIdNotEqual,
            BlockKindIdEqual,
            BlockKindIdAnd,
            BlockKindIdOr,
            BlockKindIdCall,
            BlockKindIdTable,
        },
        .blockKindIdCount = 19,
    },
    [PinKindStatement] = {
        .blockKindIds = (BlockKindId[]){
            BlockKindIdAssign,
            BlockKindIdDo,
            BlockKindIdFunction,
            BlockKindIdIf,
            BlockKindIdCall,
            BlockKindIdForLoop,
            BlockKindIdForInLoop,
            BlockKindIdWhileLoop,
            BlockKindIdReturn,
            BlockKindIdLocal,
        },
        .blockKindIdCount = 10,
    },
    [PinKindIdentifier] = {
        .blockKindIds = (BlockKindId[]){
            BlockKindIdIdentifier,
        },
        .blockKindIdCount = 1,
    },
    [PinKindTableEntry] = {
        .blockKindIds = (BlockKindId[]){
            BlockKindIdTableKeyValuePair,
            BlockKindIdTableExpressionValuePair,
            BlockKindIdTableValue,
        },
        .blockKindIdCount = 3,
    },
    [PinKindAccessModifiable] = {
        .blockKindIds = (BlockKindId[]){
            BlockKindIdAssign,
            BlockKindIdFunction,
        },
        .blockKindIdCount = 2,
    },
    [PinKindMultiExpression] = {
        .blockKindIds = (BlockKindId[]){
            BlockKindIdExpressionList,
        },
        .blockKindIdCount = 1,
        .extendsPinKind = PinKindExpression,
    },
    [PinKindNone] = {
        .blockKindIdCount = 0,
    },
};

BlockKind BlockKinds[BlockKindIdCount];

void BlockKindsInit(void)
{
    BlockKinds[BlockKindIdPin] = BlockKindNew((BlockKind){
        .text = ".",
        .save = SaverSavePin,
    });
    BlockKinds[BlockKindIdDo] = BlockKindNew((BlockKind){
        .searchText = "do",
        .text = "do",
        .isVertical = true,
        .isGrowable = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindStatement),
            },
        .defaultChildrenCount = 1,
        .save = SaverSaveDo,
    });
    BlockKinds[BlockKindIdStatementList] = BlockKindNew((BlockKind){
        .isVertical = true,
        .isGrowable = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindStatement),
            },
        .defaultChildrenCount = 1,
        .save = SaverSaveStatementList,
    });
    BlockKinds[BlockKindIdFunctionHeader] = BlockKindNew((BlockKind){
        .text = "function",
        .isGrowable = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindIdentifier),
                NewChildPin(PinKindIdentifier),
            },
        .defaultChildrenCount = 2,
        .save = SaverSaveFunctionHeader,
    });
    BlockKinds[BlockKindIdFunction] = BlockKindNew((BlockKind){
        .searchText = "function",
        .isVertical = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChild(BlockKindIdFunctionHeader),
                NewChild(BlockKindIdStatementList),
            },
        .defaultChildrenCount = 2,
        .save = SaverSaveFunction,
    });
    BlockKinds[BlockKindIdLambdaFunctionHeader] = BlockKindNew((BlockKind){
        .text = "function",
        .isGrowable = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindIdentifier),
            },
        .defaultChildrenCount = 1,
        .save = SaverSaveLambdaFunctionHeader,
    });
    BlockKinds[BlockKindIdLambdaFunction] = BlockKindNew((BlockKind){
        .searchText = "function",
        .isVertical = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChild(BlockKindIdLambdaFunctionHeader),
                NewChild(BlockKindIdStatementList),
            },
        .defaultChildrenCount = 2,
        .save = SaverSaveLambdaFunction,
    });
    BlockKinds[BlockKindIdCase] = BlockKindNew((BlockKind){
        .text = "case",
        .isVertical = true,
        .isGrowable = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindExpression),
                NewChildPin(PinKindStatement),
            },
        .defaultChildrenCount = 2,
        .save = SaverSaveCase,
    });
    BlockKinds[BlockKindIdIfCases] = BlockKindNew((BlockKind){
        .text = "if",
        .isVertical = true,
        .isGrowable = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChild(BlockKindIdCase),
            },
        .defaultChildrenCount = 1,
        .save = SaverSaveIfCases,
    });
    BlockKinds[BlockKindIdElseCase] = BlockKindNew((BlockKind){
        .text = "else",
        .isVertical = true,
        .isGrowable = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindStatement),
            },
        .defaultChildrenCount = 1,
        .save = SaverSaveElseCase,
    });
    BlockKinds[BlockKindIdIf] = BlockKindNew((BlockKind){
        .searchText = "if",
        .isVertical = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChild(BlockKindIdIfCases),
                NewChild(BlockKindIdElseCase),
            },
        .defaultChildrenCount = 2,
        .save = SaverSaveIf,
    });
    BlockKinds[BlockKindIdExpressionList] = BlockKindNew((BlockKind){
        .searchText = ",",
        .text = ",",
        .isTextInfix = true,
        .isGrowable = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindExpression),
            },
        .defaultChildrenCount = 1,
        .save = SaverSaveExpressionList,
    });
    BlockKinds[BlockKindIdAssign] = BlockKindNew((BlockKind){
        .searchText = "=",
        .text = "=",
        .isTextInfix = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindMultiExpression),
                NewChildPin(PinKindMultiExpression),
            },
        .defaultChildrenCount = 2,
        .save = SaverSaveAssign,
    });
    BlockKinds[BlockKindIdNot] = BlockKindNew((BlockKind){
        .searchText = "not",
        .text = "not",
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindExpression),
            },
        .defaultChildrenCount = 1,
        .save = SaverSaveNot,
    });
    BlockKinds[BlockKindIdLength] = BlockKindNew((BlockKind){
        .searchText = "#",
        .text = "#",
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindExpression),
            },
        .defaultChildrenCount = 1,
        .save = SaverSaveLength,
    });
    BlockKinds[BlockKindIdConcatenate] = BlockKindNew((BlockKind){
        .searchText = "..",
        .text = "..",
        .isTextInfix = true,
        .isGrowable = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindExpression),
                NewChildPin(PinKindExpression),
                NewChildPin(PinKindExpression),
            },
        .defaultChildrenCount = 3,
        .save = SaverSaveConcatenate,
    });
    BlockKinds[BlockKindIdModulo] = BlockKindNew((BlockKind){
        .searchText = "%",
        .text = "%",
        .isTextInfix = true,
        .isGrowable = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindExpression),
                NewChildPin(PinKindExpression),
                NewChildPin(PinKindExpression),
            },
        .defaultChildrenCount = 3,
        .save = SaverSaveModulo,
    });
    BlockKinds[BlockKindIdDivide] = BlockKindNew((BlockKind){
        .searchText = "/",
        .text = "/",
        .isTextInfix = true,
        .isGrowable = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindExpression),
                NewChildPin(PinKindExpression),
                NewChildPin(PinKindExpression),
            },
        .defaultChildrenCount = 3,
        .save = SaverSaveDivide,
    });
    BlockKinds[BlockKindIdMultiply] = BlockKindNew((BlockKind){
        .searchText = "*",
        .text = "*",
        .isTextInfix = true,
        .isGrowable = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindExpression),
                NewChildPin(PinKindExpression),
                NewChildPin(PinKindExpression),
            },
        .defaultChildrenCount = 3,
        .save = SaverSaveMultiply,
    });
    BlockKinds[BlockKindIdSubtract] = BlockKindNew((BlockKind){
        .searchText = "-",
        .text = "-",
        .isTextInfix = true,
        .isGrowable = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindExpression),
                NewChildPin(PinKindExpression),
                NewChildPin(PinKindExpression),
            },
        .defaultChildrenCount = 3,
        .save = SaverSaveSubtract,
    });
    BlockKinds[BlockKindIdAdd] = BlockKindNew((BlockKind){
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
        .save = SaverSaveAdd,
    });
    BlockKinds[BlockKindIdGreaterEqual] = BlockKindNew((BlockKind){
        .searchText = ">=",
        .text = ">=",
        .isTextInfix = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindExpression),
                NewChildPin(PinKindExpression),
            },
        .defaultChildrenCount = 2,
        .save = SaverSaveGreaterEqual,
    });
    BlockKinds[BlockKindIdLessEqual] = BlockKindNew((BlockKind){
        .searchText = "<=",
        .text = "<=",
        .isTextInfix = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindExpression),
                NewChildPin(PinKindExpression),
            },
        .defaultChildrenCount = 2,
        .save = SaverSaveLessEqual,
    });
    BlockKinds[BlockKindIdGreater] = BlockKindNew((BlockKind){
        .searchText = ">",
        .text = ">",
        .isTextInfix = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindExpression),
                NewChildPin(PinKindExpression),
            },
        .defaultChildrenCount = 2,
        .save = SaverSaveGreater,
    });
    BlockKinds[BlockKindIdLess] = BlockKindNew((BlockKind){
        .searchText = "<",
        .text = "<",
        .isTextInfix = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindExpression),
                NewChildPin(PinKindExpression),
            },
        .defaultChildrenCount = 2,
        .save = SaverSaveLess,
    });
    BlockKinds[BlockKindIdNotEqual] = BlockKindNew((BlockKind){
        .searchText = "!=",
        .text = "!=",
        .isTextInfix = true,
        .isGrowable = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindExpression),
                NewChildPin(PinKindExpression),
                NewChildPin(PinKindExpression),
            },
        .defaultChildrenCount = 3,
        .save = SaverSaveNotEqual,
    });
    BlockKinds[BlockKindIdEqual] = BlockKindNew((BlockKind){
        .searchText = "==",
        .text = "==",
        .isTextInfix = true,
        .isGrowable = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindExpression),
                NewChildPin(PinKindExpression),
                NewChildPin(PinKindExpression),
            },
        .defaultChildrenCount = 3,
        .save = SaverSaveEqual,
    });
    BlockKinds[BlockKindIdAnd] = BlockKindNew((BlockKind){
        .searchText = "and",
        .text = "and",
        .isTextInfix = true,
        .isGrowable = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindExpression),
                NewChildPin(PinKindExpression),
                NewChildPin(PinKindExpression),
            },
        .defaultChildrenCount = 3,
        .save = SaverSaveAnd,
    });
    BlockKinds[BlockKindIdOr] = BlockKindNew((BlockKind){
        .searchText = "or",
        .text = "or",
        .isTextInfix = true,
        .isGrowable = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindExpression),
                NewChildPin(PinKindExpression),
                NewChildPin(PinKindExpression),
            },
        .defaultChildrenCount = 3,
        .save = SaverSaveOr,
    });
    BlockKinds[BlockKindIdCall] = BlockKindNew((BlockKind){
        .searchText = "call",
        .text = "call",
        .isGrowable = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindExpression),
                NewChildPin(PinKindExpression),
            },
        .defaultChildrenCount = 2,
        .save = SaverSaveCall,
    });
    BlockKinds[BlockKindIdIdentifier] = BlockKindNew((BlockKind){
        .defaultChildrenCount = 0,
        .save = SaverSaveIdentifier,
    });
    BlockKinds[BlockKindIdForLoop] = BlockKindNew((BlockKind){
        .searchText = "for",
        .text = "for",
        .isVertical = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChild(BlockKindIdForLoopCondition),
                NewChild(BlockKindIdStatementList),
            },
        .defaultChildrenCount = 2,
        .save = SaverSaveForLoop,
    });
    BlockKinds[BlockKindIdForLoopCondition] = BlockKindNew((BlockKind){
        .text = "=",
        .isTextInfix = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindIdentifier),
                NewChild(BlockKindIdForLoopBounds),
            },
        .defaultChildrenCount = 2,
        .save = SaverSaveForLoopCondition,
    });
    BlockKinds[BlockKindIdForLoopBounds] = BlockKindNew((BlockKind){
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindExpression),
                NewChildPin(PinKindExpression),
                NewChildPin(PinKindExpression),
            },
        .defaultChildrenCount = 3,
        .save = SaverSaveForLoopBounds,
    });
    BlockKinds[BlockKindIdForInLoop] = BlockKindNew((BlockKind){
        .searchText = "for in",
        .text = "for",
        .isVertical = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChild(BlockKindIdForInLoopCondition),
                NewChild(BlockKindIdStatementList),
            },
        .defaultChildrenCount = 2,
        .save = SaverSaveForLoop,
    });
    BlockKinds[BlockKindIdForInLoopCondition] = BlockKindNew((BlockKind){
        .text = "in",
        .isTextInfix = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindIdentifier),
                NewChildPin(PinKindExpression),
            },
        .defaultChildrenCount = 2,
        .save = SaverSaveForInLoopCondition,
    });
    BlockKinds[BlockKindIdWhileLoop] = BlockKindNew((BlockKind){
        .searchText = "while",
        .text = "while",
        .isVertical = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindExpression),
                NewChild(BlockKindIdStatementList),
            },
        .defaultChildrenCount = 2,
        .save = SaverSaveWhileLoop,
    });
    BlockKinds[BlockKindIdReturn] = BlockKindNew((BlockKind){
        .searchText = "return",
        .text = "return",
        .isGrowable = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindExpression),
            },
        .defaultChildrenCount = 1,
        .save = SaverSaveReturn,
    });
    BlockKinds[BlockKindIdLocal] = BlockKindNew((BlockKind){
        .searchText = "local",
        .text = "local",
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindAccessModifiable),
            },
        .defaultChildrenCount = 1,
        .save = SaverSaveLocal,
    });
    BlockKinds[BlockKindIdTable] = BlockKindNew((BlockKind){
        .searchText = "table",
        .text = "table",
        .isVertical = true,
        .isGrowable = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindTableEntry),
            },
        .defaultChildrenCount = 1,
        .save = SaverSaveTable,
    });
    BlockKinds[BlockKindIdTableKeyValuePair] = BlockKindNew((BlockKind){
        .searchText = "=",
        .text = "=",
        .isTextInfix = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindIdentifier),
                NewChildPin(PinKindExpression),
            },
        .defaultChildrenCount = 2,
        .save = SaverSaveTableKeyValuePair,
    });
    BlockKinds[BlockKindIdTableExpressionValuePair] = BlockKindNew((BlockKind){
        .searchText = "[]",
        .text = "[]",
        .isTextInfix = true,
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindExpression),
                NewChildPin(PinKindExpression),
            },
        .defaultChildrenCount = 2,
        .save = SaverSaveTableExpressionValuePair,
    });
    BlockKinds[BlockKindIdTableValue] = BlockKindNew((BlockKind){
        .searchText = ",",
        .defaultChildren =
            (DefaultChildKind[]){
                NewChildPin(PinKindExpression),
            },
        .defaultChildrenCount = 1,
        .save = SaverSaveTableValue,
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
            FontGetTextSize(BlockKinds[i].text, &BlockKinds[i].textWidth, &BlockKinds[i].textHeight, NULL, NULL, font);
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
        // Growable children don't need to be created by default, unless they're the only child.
        if (i != 0 && i == kind->defaultChildrenCount - 1 && kind->isGrowable)
        {
            continue;
        }

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

    FontGetTextSize(identifierData->text, &identifierData->textWidth, &identifierData->textHeight, NULL, NULL, font);

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
            Block *child = BlockGetChild(block, i);

            BlockDelete(child);
        }

        ListDelete_BlockPointer(&block->data.parent.children);
    }

    free(block);
}

void BlockMarkNeedsUpdate(Block *block)
{
    block->y = INT32_MAX;

    Block *parent = block->parent;

    while (parent && parent->y != INT32_MAX)
    {
        parent->y = INT32_MAX;
        parent = parent->parent;
    }
}

bool BlockContainsNonPin(Block *block)
{
    int32_t childrenCount = BlockGetChildrenCount(block);

    for (int32_t i = 0; i < childrenCount; i++)
    {
        Block *child = BlockGetChild(block, i);

        if (child->kindId != BlockKindIdPin)
        {
            return true;
        }
    }

    return false;
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

DefaultChildKind *BlockGetDefaultChildKind(Block *block, int32_t childI)
{
    const BlockKind *kind = &BlockKinds[block->kindId];
    childI = MathInt32Min(childI, kind->defaultChildrenCount - 1);

    return &kind->defaultChildren[childI];
}

Block *BlockGetChild(Block *block, int32_t childI)
{
    return block->data.parent.children.data[childI];
}

void BlockGetGlobalPosition(Block *block, int32_t *x, int32_t *y)
{
    *x = block->x;
    *y = block->y;

    Block *parent = block->parent;

    while (parent)
    {
        *x += parent->x;
        *y += parent->y;

        parent = parent->parent;
    }
}

// TODO: Unify this function with the cursor's pin kind iterator function if possible.
// TODO: Can we remove .pinKind from block kinds?
bool BlockCanPinKindContainBlockKind(BlockKindId blockKindId, PinKind pinKind)
{
    const PinKindValidBlockSet *validBlocks = &PinKindValidBlocks[pinKind];

    if (validBlocks->extendsPinKind != PinKindNone)
    {
        if (BlockCanPinKindContainBlockKind(blockKindId, validBlocks->extendsPinKind))
        {
            return true;
        }
    }

    for (int32_t i = 0; i < validBlocks->blockKindIdCount; i++)
    {
        BlockKindId kindId = validBlocks->blockKindIds[i];

        if (blockKindId == kindId)
        {
            return true;
        }
    }

    return false;
}

static bool BlockCanSwapInto(Block *block, DefaultChildKind *destinationDefaultChildKind)
{
    if (!destinationDefaultChildKind->isPin)
    {
        return block->kindId == destinationDefaultChildKind->blockKindId;
    }

    return block->kindId == BlockKindIdPin || BlockCanPinKindContainBlockKind(block->kindId, destinationDefaultChildKind->pinKind);
}

bool BlockCanSwapWith(Block *block, Block *other)
{
    bool canBlockSwapWithOther = true;

    if (other->parent)
    {
        DefaultChildKind *otherDefaultChildKind = BlockGetDefaultChildKind(other->parent, other->childI);
        canBlockSwapWithOther = BlockCanSwapInto(block, otherDefaultChildKind);
    }

    bool canOtherSwapWithBlock = true;

    if (block->parent)
    {
        DefaultChildKind *defaultChildKind = BlockGetDefaultChildKind(block->parent, block->childI);
        canOtherSwapWithBlock = BlockCanSwapInto(other, defaultChildKind);
    }

    return canBlockSwapWithOther && canOtherSwapWithBlock;
}

// Frees the old child if it exists, expands this block's children array as necessary.
// If there was already a child at this location, either delete it or return it,
// depending on doDelete.
Block *BlockReplaceChild(Block *block, Block *child, int32_t childI, bool doDelete)
{
    assert(block->kindId != BlockKindIdIdentifier);

    BlockParentData *parentData = &block->data.parent;
    Block *oldChild = parentData->children.data[childI];

    if (childI >= parentData->children.count)
    {
        childI = parentData->children.count;
        ListPush_BlockPointer(&parentData->children, (BlockPointer){0});
    }
    else if (doDelete)
    {
        BlockDelete(oldChild);
        oldChild = NULL;
    }

    parentData->children.data[childI] = child;
    child->parent = block;
    child->childI = childI;

    return oldChild;
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

// Returns a BlockDeleteResult, which will contain the previous child if doDelete is false.
// wasRemoved is true if the child was fully removed, or false if it was replaced by a default.
BlockDeleteResult BlockDeleteChild(Block *block, int32_t childI, bool doDelete)
{
    BlockKind *kind = &BlockKinds[block->kindId];
    BlockParentData *parentData = &block->data.parent;
    Block *oldChild = parentData->children.data[childI];

    if (kind->isGrowable && childI != 0 && childI >= kind->defaultChildrenCount - 1)
    {
        // This isn't a default child, so it doesn't need to be preserved. Fully delete it.
        if (doDelete)
        {
            BlockDelete(oldChild);
            oldChild = NULL;
        }

        ListRemove_BlockPointer(&parentData->children, childI);

        for (int32_t i = childI; i < parentData->children.count; i++)
        {
            parentData->children.data[i]->childI -= 1;
        }

        return (BlockDeleteResult){
            .oldChild = oldChild,
            .wasRemoved = true,
        };
    }

    // This is a default child, so it needs to be preserved. Replace it with
    // it's default value instead of deleting it.
    DefaultChildKind *defaultKind = BlockGetDefaultChildKind(block, childI);

    Block *defaultBlock = BlockNew(defaultKind->blockKindId, block, childI);
    BlockReplaceChild(block, defaultBlock, childI, doDelete);

    return (BlockDeleteResult){
        .oldChild = oldChild,
        .wasRemoved = false,
    };
}

void BlockSwapChildren(Block *block, int32_t firstChildI, int32_t secondChildI)
{
    BlockParentData *parentData = &block->data.parent;
    Block *firstChild = parentData->children.data[firstChildI];
    Block *secondChild = parentData->children.data[secondChildI];

    if (!BlockCanSwapWith(firstChild, secondChild))
    {
        return;
    }

    firstChild->childI = secondChildI;
    secondChild->childI = firstChildI;

    parentData->children.data[firstChildI] = secondChild;
    parentData->children.data[secondChildI] = firstChild;
}

uint64_t BlockCountAll(Block *block)
{
    uint64_t count = 1;

    for (int32_t i = 0; i < BlockGetChildrenCount(block); i++)
    {
        Block *child = BlockGetChild(block, i);

        count += BlockCountAll(child);
    }

    return count;
}

void BlockUpdateTree(Block *block, int32_t x, int32_t y)
{
    bool needsUpdate = block->y == INT32_MAX;

    block->x = x;
    block->y = y;

    if (!needsUpdate)
    {
        return;
    }

    int32_t textWidth, textHeight;
    BlockGetTextSize(block, &textWidth, &textHeight);

    int32_t childrenCount = BlockGetChildrenCount(block);

    int32_t localX = 0;
    int32_t localY = 0;

    localX += BlockPadding;

    if (childrenCount > 0)
    {
        localY += BlockPadding;
    }
    else
    {
        localY += textHeight;
    }

    const BlockKind *kind = &BlockKinds[block->kindId];

    if (!kind->isTextInfix)
    {
        localX += textWidth;

        if (textWidth != 0)
        {
            localX += BlockPadding;
        }
    }

    int32_t startX = localX;
    int32_t maxWidth = 0;

    if (kind->isVertical)
    {
        for (int32_t i = 0; i < childrenCount; i++)
        {
            Block *child = BlockGetChild(block, i);

            BlockUpdateTree(child, localX, localY);

            localX += child->width + BlockPadding;
            localY += child->height + BlockPadding;

            maxWidth = MathInt32Max(maxWidth, localX);
            localX = startX;
        }
    }
    else
    {
        int32_t maxHeight = 0;

        for (int32_t i = 0; i < childrenCount; i++)
        {
            Block *child = BlockGetChild(block, i);

            BlockUpdateTree(child, localX, localY);

            localX += child->width + BlockPadding;

            if (i < childrenCount - 1 && kind->isTextInfix)
            {
                localX += textWidth + BlockPadding;
            }

            maxHeight = MathInt32Max(maxHeight, child->height + BlockPadding);
        }

        maxWidth = MathInt32Max(maxWidth, localX);
        localX = startX;
        localY += maxHeight;
    }

    block->width = maxWidth;
    block->height = localY;
}

static Color BlockGetDepthColor(int32_t depth, Theme *theme)
{
    if (depth % 2 == 0)
    {
        return theme->evenColor;
    }

    return theme->oddColor;
}

static int32_t BlockFindFirstVisibleChildI(Block *block, int32_t childrenCount, Camera *camera, int32_t y)
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
        Block *child = BlockGetChild(block, i);

        if (y + child->y + child->height < camera->y)
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

void BlockDraw(Block *block, Block *cursorBlock, int32_t depth, Camera *camera, Font *font, Theme *theme, int32_t x, int32_t y)
{
    x += block->x;
    y += block->y;

    if (block->kindId == BlockKindIdPin)
    {
        ColorSet(theme->pinColor);
    }
    else
    {
        ColorSet(BlockGetDepthColor(depth, theme));
    }

    DrawRect((float)x - BlockPadding, (float)y - BlockPadding, (float)block->width, (float)block->height, camera->zoom);

    ColorSet(theme->textColor);

    int32_t textY = y - BlockPadding / 2;
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
        FontDraw(text, x * camera->zoom, textY * camera->zoom, font);
    }

    if (!hasChildren)
    {
        return;
    }

    int32_t firstVisibleI = BlockFindFirstVisibleChildI(block, childrenCount, camera, y);

    for (int32_t i = firstVisibleI; i < childrenCount; i++)
    {
        Block *child = BlockGetChild(block, i);

        if (y + child->y > camera->y + camera->height / camera->zoom)
        {
            break;
        }

        BlockDraw(child, cursorBlock, depth + 1, camera, font, theme, x, y);

        if (i < childrenCount - 1 && kind->isTextInfix)
        {
            FontDraw(text, (x + child->x + child->width) * camera->zoom, textY * camera->zoom, font);
        }
    }
}