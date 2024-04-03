#include "Cursor.h"
#include "Shapes.h"
#include "Math.h"

#include <GLFW/glfw3.h>

static const float LineWidth = 3;
static const float AnimationSpeed = 30.0f;

Cursor CursorNew(Block *block)
{
    return (Cursor){
        .block = block,
        .insertText = ListNew_char(16),
        .state = CursorStateMove,
        .isFirstDraw = true,
    };
}

void CursorDelete(Cursor *cursor)
{
    ListDelete_char(&cursor->insertText);

    if (cursor->clipboardBlock)
    {
        BlockDelete(cursor->clipboardBlock);
    }
}

static bool CursorIsVertical(Cursor *cursor)
{
    BlockKind *parentKind = &BlockKinds[cursor->block->kindId];

    if (cursor->block->parent)
    {
        parentKind = &BlockKinds[cursor->block->parent->kindId];
    }

    return parentKind->isVertical;
}

static bool CursorGetChildIndexInDirection(Cursor *cursor, InsertDirection direction, int32_t *childI)
{
    *childI = cursor->block->childI;

    if (CursorIsVertical(cursor))
    {
        if (direction == InsertDirectionLeft || direction == InsertDirectionRight)
        {
            return false;
        }

        if (direction == InsertDirectionDown)
        {
            *childI += 1;
        }
        else if (direction == InsertDirectionUp)
        {
            *childI -= 1;
        }
    }
    else
    {
        if (direction == InsertDirectionUp || direction == InsertDirectionDown)
        {
            return false;
        }

        if (direction == InsertDirectionRight)
        {
            *childI += 1;
        }
        else if (direction == InsertDirectionLeft)
        {
            *childI -= 1;
        }
    }

    return true;
}

static bool CursorGetChildInsertIndexInDirection(Cursor *cursor, int32_t *childI)
{
    *childI = cursor->block->childI;

    if (CursorIsVertical(cursor))
    {
        if (cursor->insertDirection == InsertDirectionLeft || cursor->insertDirection == InsertDirectionRight)
        {
            return false;
        }

        if (cursor->insertDirection == InsertDirectionDown)
        {
            *childI += 1;
        }
    }
    else
    {
        if (cursor->insertDirection == InsertDirectionUp || cursor->insertDirection == InsertDirectionDown)
        {
            return false;
        }

        if (cursor->insertDirection == InsertDirectionRight)
        {
            *childI += 1;
        }
    }

    return true;
}

static void CursorAddChild(Cursor *cursor, Block *parent, Block *child, int32_t childI, Tree *tree)
{
    TreeRequestUpdate(tree, cursor->block->y);

    if (cursor->insertDirection == InsertDirectionCenter)
    {
        BlockReplaceChild(parent, child, childI);
    }
    else
    {
        BlockInsertChild(parent, child, childI);
    }

    cursor->block = child;
}

static void CursorEndInsert(Cursor *cursor)
{
    cursor->state = CursorStateMove;
    ListReset_char(&cursor->insertText);
}

static void CursorStartInsert(Cursor *cursor, InsertDirection insertDirection, Tree *tree)
{
    if (!cursor->block->parent)
    {
        return;
    }

    cursor->insertDirection = insertDirection;

    int32_t childI;
    if (!CursorGetChildInsertIndexInDirection(cursor, &childI))
    {
        return;
    }

    const BlockKind *kind = &BlockKinds[cursor->block->parent->kindId];

    if (insertDirection != InsertDirectionCenter && (!kind->isGrowable || childI < kind->defaultChildrenCount - 1))
    {
        return;
    }

    cursor->state = CursorStateInsert;

    const DefaultChildKind *defaultChildKind = BlockGetDefaultChild(cursor->block->parent, cursor->block->childI);
    Block *parent = cursor->block->parent;

    if (parent && !defaultChildKind->isPin)
    {
        Block *block = BlockNew(defaultChildKind->blockKindId, parent, childI);
        CursorAddChild(cursor, parent, block, childI, tree);
        CursorEndInsert(cursor);

        return;
    }
}

static void CursorShift(Cursor *cursor, InsertDirection shiftDirection, Tree *tree)
{
    if (!cursor->block->parent)
    {
        return;
    }

    BlockParentData *parentParentData = &cursor->block->parent->data.parent;

    int32_t childI;
    if (!CursorGetChildIndexInDirection(cursor, shiftDirection, &childI))
    {
        return;
    }

    childI = MathInt32Clamp(childI, 0, parentParentData->children.count - 1);

    DefaultChildKind *otherDefaultChildKind = BlockGetDefaultChild(cursor->block->parent, childI);

    if (!BlockCanSwapWith(cursor->block, otherDefaultChildKind))
    {
        return;
    }

    TreeRequestUpdate(tree, cursor->block->y);
    TreeRequestUpdate(tree, parentParentData->children.data[childI]->y);

    parentParentData->children.data[cursor->block->childI] = parentParentData->children.data[childI];
    parentParentData->children.data[childI]->childI = cursor->block->childI;

    parentParentData->children.data[childI] = cursor->block;
    cursor->block->childI = childI;
}

static void CursorCopy(Cursor *cursor)
{
    if (!cursor->block->parent)
    {
        return;
    }

    free(cursor->clipboardBlock);
    cursor->clipboardBlock = BlockCopy(cursor->block, NULL, 0);
    cursor->clipboardDefaultChildKind = BlockGetDefaultChild(cursor->block->parent, cursor->block->childI);
}

static void CursorCut(Cursor *cursor, Tree *tree)
{
    CursorCopy(cursor);
    CursorDeleteHere(cursor, tree);
}

static void CursorPaste(Cursor *cursor, Tree *tree)
{
    if (!cursor->block->parent || !cursor->clipboardBlock)
    {
        return;
    }

    if (!BlockCanSwapWith(cursor->block, cursor->clipboardDefaultChildKind))
    {
        return;
    }

    BlockParentData *parentParentData = &cursor->block->parent->data.parent;
    Block *pastedBlock = BlockCopy(cursor->clipboardBlock, cursor->block->parent, cursor->block->childI);

    TreeRequestUpdate(tree, cursor->block->y);

    BlockReplaceChild(cursor->block->parent, pastedBlock, cursor->block->childI);

    cursor->block = pastedBlock;

}

static void CursorUpdateMove(Cursor *cursor, Input *input, Tree *tree)
{
    bool isShiftHeld = InputIsButtonHeld(input, GLFW_KEY_LEFT_SHIFT) || InputIsButtonHeld(input, GLFW_KEY_RIGHT_SHIFT);

    if (isShiftHeld)
    {
        if (InputIsButtonPressedOrRepeat(input, GLFW_KEY_E))
        {
            CursorShift(cursor, InsertDirectionUp, tree);
        }

        if (InputIsButtonPressedOrRepeat(input, GLFW_KEY_D))
        {
            CursorShift(cursor, InsertDirectionDown, tree);
        }

        if (InputIsButtonPressedOrRepeat(input, GLFW_KEY_S))
        {
            CursorShift(cursor, InsertDirectionLeft, tree);
        }

        if (InputIsButtonPressedOrRepeat(input, GLFW_KEY_F))
        {
            CursorShift(cursor, InsertDirectionRight, tree);
        }
    }
    else
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
    }

    if (InputIsButtonPressedOrRepeat(input, GLFW_KEY_BACKSPACE))
    {
        CursorDeleteHere(cursor, tree);
    }

    if (InputIsButtonPressed(input, GLFW_KEY_SPACE))
    {
        CursorStartInsert(cursor, InsertDirectionCenter, tree);
    }

    if (InputIsButtonPressed(input, GLFW_KEY_I))
    {
        CursorStartInsert(cursor, InsertDirectionUp, tree);
    }

    if (InputIsButtonPressed(input, GLFW_KEY_K))
    {
        CursorStartInsert(cursor, InsertDirectionDown, tree);
    }

    if (InputIsButtonPressed(input, GLFW_KEY_J))
    {
        CursorStartInsert(cursor, InsertDirectionLeft, tree);
    }

    if (InputIsButtonPressed(input, GLFW_KEY_L))
    {
        CursorStartInsert(cursor, InsertDirectionRight, tree);
    }

    if (InputIsButtonPressed(input, GLFW_KEY_X))
    {
        CursorCut(cursor, tree);
    }

    if (InputIsButtonPressed(input, GLFW_KEY_C))
    {
        CursorCopy(cursor);
    }

    if (InputIsButtonPressed(input, GLFW_KEY_V))
    {
        CursorPaste(cursor, tree);
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

    return string[list->count] == '\0';
}

static void CursorUpdateInsert(Cursor *cursor, Input *input, Tree *tree, Font *font)
{
    bool isCanceling = InputIsButtonPressed(input, GLFW_KEY_ESCAPE);
    bool isConfirming = InputIsButtonPressed(input, GLFW_KEY_ENTER);

    int32_t childI;
    CursorGetChildInsertIndexInDirection(cursor, &childI);

    const DefaultChildKind *defaultChildKind = BlockGetDefaultChild(cursor->block->parent, childI);
    Block *parent = cursor->block->parent;

    if (isConfirming && parent && defaultChildKind->isPin)
    {
        const PinKindInsertBlocks *insertBlocks = &PinInsertBlocks[defaultChildKind->pinKind];

        if (cursor->insertText.count == 0 && defaultChildKind->isPin)
        {
            Block *block = BlockNew(BlockKindIdPin, parent, childI);
            CursorAddChild(cursor, parent, block, childI, tree);
            CursorEndInsert(cursor);

            return;
        }

        for (int32_t i = 0; i < insertBlocks->blockKindIdCount; i++)
        {
            BlockKindId kindId = insertBlocks->blockKindIds[i];
            const BlockKind *kind = &BlockKinds[kindId];

            if (!ListMatchesString(&cursor->insertText, kind->searchText))
            {
                continue;
            }

            Block *block = BlockNew(kindId, parent, childI);
            CursorAddChild(cursor, parent, block, childI, tree);
            CursorEndInsert(cursor);

            return;
        }

        if (defaultChildKind->pinKind == PinKindIdentifier || defaultChildKind->pinKind == PinKindExpression)
        {
            Block *block = BlockNewIdentifier(cursor->insertText.data, cursor->insertText.count, font, parent, childI);
            CursorAddChild(cursor, parent, block, childI, tree);
            CursorEndInsert(cursor);

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

void CursorUpdate(Cursor *cursor, Input *input, Tree *tree, Font *font)
{
    switch (cursor->state)
    {
    case CursorStateMove: {
        CursorUpdateMove(cursor, input, tree);
        break;
    }
    case CursorStateInsert: {
        CursorUpdateInsert(cursor, input, tree, font);
        break;
    }
    }
}

void CursorDraw(Cursor *cursor, Theme *theme, float deltaTime)
{
    ColorSet(theme->cursorColor);

    Block *block = cursor->block;

    float targetX = block->x - BlockPadding - LineWidth;
    float targetY = block->y - BlockPadding - LineWidth;
    float targetWidth = block->width + LineWidth * 2;
    float targetHeight = block->height + LineWidth * 2;

    if (cursor->isFirstDraw)
    {
        cursor->x = targetX;
        cursor->y = targetY;
        cursor->width = targetWidth;
        cursor->height = targetHeight;

        cursor->isFirstDraw = false;
    }

    float delta = deltaTime * AnimationSpeed;
    cursor->x = MathLerp(cursor->x, targetX, delta);
    cursor->y = MathLerp(cursor->y, targetY, delta);
    cursor->width = MathLerp(cursor->width, targetWidth, delta);
    cursor->height = MathLerp(cursor->height, targetHeight, delta);

    DrawRect(cursor->x, cursor->y, cursor->width, LineWidth);
    DrawRect(cursor->x, cursor->y + cursor->height - LineWidth, cursor->width, LineWidth);
    DrawRect(cursor->x, cursor->y, LineWidth, cursor->height);
    DrawRect(cursor->x + cursor->width - LineWidth, cursor->y, LineWidth, cursor->height);
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
    if (CursorIsVertical(cursor))
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
void CursorDeleteHere(Cursor *cursor, Tree *tree)
{
    if (!cursor->block->parent)
    {
        return;
    }

    BlockKind *parentKind = &BlockKinds[cursor->block->parent->kindId];
    BlockParentData *parentParentData = &cursor->block->parent->data.parent;

    TreeRequestUpdate(tree, cursor->block->y);

    if (parentKind->isGrowable && cursor->block->childI >= parentKind->defaultChildrenCount)
    {
        // This isn't a default child, so it doesn't need to be preserved. Fully delete it.
        int32_t deleteI = cursor->block->childI;

        if (cursor->block->childI < parentParentData->children.count - 1)
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
}