#include "Cursor.h"

#include <GLFW/glfw3.h>

Cursor CursorNew(Block *block)
{
    return (Cursor){
        .block = block,
        .insertText = ListNew_char(16),
        .state = CursorStateMove,
    };
}

void CursorDelete(Cursor *cursor)
{
    ListDelete_char(&cursor->insertText);
}

static void CursorStartInsert(Cursor *cursor, InsertDirection insertDirection)
{
    cursor->state = CursorStateInsert;
    cursor->insertDirection = insertDirection;

    const DefaultChildKind *defaultChildKind = BlockGetDefaultChild(cursor->block->parent, cursor->block->childI);
    Block *parent = cursor->block->parent;

    if (parent && !defaultChildKind->isPin)
    {
        int32_t childI = cursor->block->childI;

        BlockReplaceChild(parent, BlockNew(defaultChildKind->blockKindId, parent, childI), childI);
        cursor->state = CursorStateMove;
    }
}

static void CursorUpdateMove(Cursor *cursor, Input *input, Block *rootBlock)
{
    if (InputIsButtonPressedOrRepeat(input, GLFW_KEY_E))
    {
        CursorUp(cursor);
    }

    if (InputIsButtonPressedOrRepeat(input, GLFW_KEY_D))
    {
        CursorDown(cursor);
    }

    if (InputIsButtonPressedOrRepeat(input, GLFW_KEY_S))
    {
        CursorLeft(cursor);
    }

    if (InputIsButtonPressedOrRepeat(input, GLFW_KEY_F))
    {
        CursorRight(cursor);
    }

    if (InputIsButtonPressedOrRepeat(input, GLFW_KEY_BACKSPACE))
    {
        CursorDeleteHere(cursor, rootBlock);
    }

    if (InputIsButtonPressed(input, GLFW_KEY_SPACE))
    {
        CursorStartInsert(cursor, InsertDirectionCenter);
    }

    if (InputIsButtonPressed(input, GLFW_KEY_I))
    {
        CursorStartInsert(cursor, InsertDirectionUp);
    }

    if (InputIsButtonPressed(input, GLFW_KEY_K))
    {
        CursorStartInsert(cursor, InsertDirectionDown);
    }

    if (InputIsButtonPressed(input, GLFW_KEY_J))
    {
        CursorStartInsert(cursor, InsertDirectionLeft);
    }

    if (InputIsButtonPressed(input, GLFW_KEY_L))
    {
        CursorStartInsert(cursor, InsertDirectionRight);
    }
}

static bool ListMatchesString(List_char *list, char *string)
{
    if (!string)
    {
        return false;
    }

    for (int32_t i = 0; i < list->count; i++)
    {
        if (string[i] == '\0' || list->data[i] != string[i])
        {
            return false;
        }
    }

    return true;
}

static void CursorUpdateInsert(Cursor *cursor, Input *input, Block *rootBlock, Font *font)
{
    bool isCanceling = InputIsButtonPressed(input, GLFW_KEY_ESCAPE);
    bool isConfirming = InputIsButtonPressed(input, GLFW_KEY_ENTER);

    const DefaultChildKind *defaultChildKind = BlockGetDefaultChild(cursor->block->parent, cursor->block->childI);
    Block *parent = cursor->block->parent;

    if (isConfirming && parent && defaultChildKind->isPin)
    {
        const PinKindInsertBlocks *insertBlocks = &PinInsertBlocks[defaultChildKind->pinKind];

        for (int32_t i = 0; i < insertBlocks->blockKindIdCount; i++)
        {
            BlockKindId kindId = insertBlocks->blockKindIds[i];
            const BlockKind *kind = &BlockKinds[kindId];

            if (!ListMatchesString(&cursor->insertText, kind->searchText))
            {
                continue;
            }

            int32_t childI = cursor->block->childI;
            Block *block = BlockNew(kindId, parent, childI);

            BlockReplaceChild(parent, block, childI);
            cursor->block = block;
            BlockUpdateTree(rootBlock, rootBlock->x, rootBlock->y);

            cursor->state = CursorStateMove;
            ListReset_char(&cursor->insertText);

            return;
        }

        if (defaultChildKind->pinKind == PinKindIdentifier || defaultChildKind->pinKind == PinKindExpression)
        {
            int32_t childI = cursor->block->childI;
            Block *block = BlockNewIdentifier(cursor->insertText.data, cursor->insertText.count, font, parent, childI);

            BlockReplaceChild(parent, block, childI);
            cursor->block = block;
            BlockUpdateTree(rootBlock, rootBlock->x, rootBlock->y);

            cursor->state = CursorStateMove;
            ListReset_char(&cursor->insertText);

            return;
        }
    }

    if (isCanceling)
    {
        cursor->state = CursorStateMove;
        ListReset_char(&cursor->insertText);

        return;
    }

    for (int32_t i = 0; i < input->typedChars.count; i++)
    {
        ListPush_char(&cursor->insertText, input->typedChars.data[i]);
    }
}

void CursorUpdate(Cursor *cursor, Input *input, Block *rootBlock, Font *font)
{
    switch (cursor->state)
    {
        case CursorStateMove: {
            CursorUpdateMove(cursor, input, rootBlock);
            break;
        }
        case CursorStateInsert: {
            CursorUpdateInsert(cursor, input, rootBlock, font);
            break;
        }
    }
}

void CursorAscend(Cursor *cursor)
{
    if (!cursor->block->parent)
    {
        return;
    }

    cursor->block = cursor->block->parent;
}

void CursorDescend(Cursor *cursor)
{
    if (BlockGetChildrenCount(cursor->block) < 1)
    {
        return;
    }

    cursor->block = cursor->block->data.parent.children.data[0];
}

static void CursorMove(Cursor *cursor, int32_t delta)
{
    if (!cursor->block->parent)
    {
        return;
    }

    int32_t parentChildrenCount = BlockGetChildrenCount(cursor->block->parent);

    if (BlockGetChildrenCount(cursor->block->parent) < 1)
    {
        return;
    }

    int32_t nextI = (cursor->block->childI + delta);

    // Make nextI positive so that it will always wrap correctly.
    while (nextI < 0)
    {
        nextI += parentChildrenCount;
    }

    nextI %= parentChildrenCount;
    cursor->block = cursor->block->parent->data.parent.children.data[nextI];
}

void CursorNext(Cursor *cursor)
{
    CursorMove(cursor, 1);
}

void CursorPrevious(Cursor *cursor)
{
    CursorMove(cursor, -1);
}

static void CursorMoveHorizontalOrVertical(
    Cursor *cursor, void (*horizontalMovement)(Cursor *cursor), void (*verticalMovement)(Cursor *cursor))
{
    BlockKind *parentKind = &BlockKinds[cursor->block->kindId];

    if (cursor->block->parent)
    {
        parentKind = &BlockKinds[cursor->block->parent->kindId];
    }

    if (parentKind->isVertical)
    {
        verticalMovement(cursor);
        return;
    }

    horizontalMovement(cursor);
}

void CursorUp(Cursor *cursor)
{
    CursorMoveHorizontalOrVertical(cursor, CursorAscend, CursorPrevious);
}

void CursorDown(Cursor *cursor)
{
    CursorMoveHorizontalOrVertical(cursor, CursorDescend, CursorNext);
}

void CursorLeft(Cursor *cursor)
{
    CursorMoveHorizontalOrVertical(cursor, CursorPrevious, CursorAscend);
}

void CursorRight(Cursor *cursor)
{
    CursorMoveHorizontalOrVertical(cursor, CursorNext, CursorDescend);
}

// TODO: Most of this logic should be part of the block, it should know how to delete one of it's children.
void CursorDeleteHere(Cursor *cursor, Block *rootBlock)
{
    if (!cursor->block->parent)
    {
        return;
    }

    BlockKind *parentKind = &BlockKinds[cursor->block->parent->kindId];
    BlockParentData *parentParentData = &cursor->block->parent->data.parent;

    if (parentKind->isGrowable && cursor->block->childI >= parentKind->defaultChildrenCount)
    {
        // This isn't a default child, so it doesn't need to be preserved. Fully delete it.
        int32_t deleteI = cursor->block->childI;

        if (cursor->block->childI < parentParentData->children.count)
        {
            cursor->block = parentParentData->children.data[cursor->block->childI + 1];
        }
        else if (cursor->block->childI > 0)
        {
            cursor->block = parentParentData->children.data[cursor->block->childI - 1];
        }
        else
        {
            cursor->block = cursor->block->parent;
        }

        BlockDelete(parentParentData->children.data[deleteI]);
        ListRemove_BlockPointer(&parentParentData->children, deleteI);

        for (int32_t i = deleteI; i < parentParentData->children.count; i++)
        {
            parentParentData->children.data[i]->childI -= 1;
        }
    }
    else
    {
        // This is a default child, so it needs to be preserved. Replace it with it's default value instead of deleting
        // it.
        DefaultChildKind *defaultKind = BlockGetDefaultChild(cursor->block->parent, cursor->block->childI);

        Block *parent = cursor->block->parent;
        int32_t childI = cursor->block->childI;

        Block *defaultBlock = BlockNew(defaultKind->blockKindId, parent, childI);
        BlockReplaceChild(parent, defaultBlock, childI);

        cursor->block = defaultBlock;
    }

    BlockUpdateTree(rootBlock, rootBlock->x, rootBlock->y);
}