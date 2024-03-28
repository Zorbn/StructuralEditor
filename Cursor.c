#include "Cursor.h"

Cursor CursorNew(Block *block)
{
    return (Cursor){
        .block = block,
    };
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

static void CursorMoveHorizontalOrVertical(Cursor *cursor, void (*horizontalMovement)(Cursor *cursor), void (*verticalMovement)(Cursor *cursor))
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
        // This is a default child, so it needs to be preserved. Replace it with it's default value instead of deleting it.
        DefaultChildKind *defaultKind = BlockGetDefaultChild(cursor->block->parent, cursor->block->childI);

        Block *parent = cursor->block->parent;
        int32_t childI = cursor->block->childI;

        Block *defaultBlock = BlockNew(defaultKind->blockKindId, parent, childI);
        BlockReplaceChild(parent, defaultBlock, childI);

        cursor->block = defaultBlock;
    }

    BlockUpdateTree(rootBlock, rootBlock->x, rootBlock->y);
}