#include "Cursor.h"
#include "Math.h"
#include "Shapes.h"

#include <GLFW/glfw3.h>

static const float AnimationSpeed = 30.0f;

Cursor CursorNew(Block *block)
{
    return (Cursor){
        .block = block,
        .searchBar = SearchBarNew(),
        .state = CursorStateMove,
        .isFirstDraw = true,
        .commands = ListNew_Command(16),
    };
}

static void CommandDelete(Command *command)
{
    switch (command->kind)
    {
    case CommandKindReplace: {
        BlockDelete(command->data.replace.oldChild);

        break;
    }
    case CommandKindDelete: {
        BlockDelete(command->data.delete.oldChild);

        break;
    }
    }
}

void CursorDelete(Cursor *cursor)
{
    for (int32_t i = 0; i < cursor->commands.count; i++)
    {
        CommandDelete(&cursor->commands.data[i]);
    }

    ListDelete_Command(&cursor->commands);

    SearchBarDelete(&cursor->searchBar);

    if (cursor->clipboardBlock)
    {
        BlockDelete(cursor->clipboardBlock);
    }
}

static void CommandInsertChild(Cursor *cursor, Block *parent, Block *child, int32_t childI)
{
    Block *cursorBlock = cursor->block;

    BlockInsertChild(parent, child, childI);

    Command command = (Command){
        .kind = CommandKindInsert,
        .data.insert =
            (CommandInsertData){
                .parent = parent,
                .childI = childI,
            },
        .cursorBlock = cursorBlock,
    };

    ListPush_Command(&cursor->commands, command);
}

static void CommandReplaceChild(Cursor *cursor, Block *parent, Block *child, int32_t childI)
{
    Block *cursorBlock = cursor->block;
    Block *oldChild = BlockReplaceChild(parent, child, childI, false);

    Command command = (Command){
        .kind = CommandKindReplace,
        .data.replace =
            (CommandReplaceData){
                .parent = parent,
                .oldChild = oldChild,
                .childI = childI,
            },
        .cursorBlock = cursorBlock,
    };

    ListPush_Command(&cursor->commands, command);
}

static BlockDeleteResult CommandDeleteChild(Cursor *cursor, Block *parent, int32_t childI)
{
    Block *cursorBlock = cursor->block;
    BlockDeleteResult deleteResult = BlockDeleteChild(parent, childI, false);

    Command command = (Command){
        .kind = CommandKindDelete,
        .data.delete =
            (CommandDeleteData){
                .parent = parent,
                .oldChild = deleteResult.oldChild,
                .childI = childI,
                .wasRemoved = deleteResult.wasRemoved,
            },
        .cursorBlock = cursorBlock,
    };

    ListPush_Command(&cursor->commands, command);

    return deleteResult;
}

static void CommandSwapChildren(Cursor *cursor, Block *parent, int32_t firstChildI, int32_t secondChildI)
{
    Block *cursorBlock = cursor->block;
    BlockSwapChildren(parent, firstChildI, secondChildI);

    Command command = (Command){
        .kind = CommandKindSwap,
        .data.swap =
            (CommandSwapData){
                .parent = parent,
                .firstChildI = firstChildI,
                .secondChildI = secondChildI,
            },
        .cursorBlock = cursorBlock,
    };

    ListPush_Command(&cursor->commands, command);
}

static void CommandUndo(Cursor *cursor, Command *command)
{
    switch (command->kind)
    {
    case CommandKindInsert: {
        BlockDeleteChild(command->data.insert.parent, command->data.insert.childI, true);
        BlockMarkNeedsUpdate(command->data.insert.parent);

        break;
    }
    case CommandKindReplace: {
        if (!command->data.replace.oldChild)
        {
            break;
        }

        BlockReplaceChild(
            command->data.replace.parent, command->data.replace.oldChild, command->data.replace.childI, true);
        BlockMarkNeedsUpdate(command->data.replace.oldChild);

        break;
    }
    case CommandKindDelete: {
        if (command->data.delete.wasRemoved)
        {
            BlockInsertChild(command->data.delete.parent, command->data.delete.oldChild, command->data.delete.childI);
        }
        else
        {
            BlockReplaceChild(command->data.delete.parent, command->data.delete.oldChild, command->data.delete.childI, true);
        }

        BlockMarkNeedsUpdate(command->data.delete.oldChild);

        break;
    }
    case CommandKindSwap: {
        BlockParentData *parentData = &command->data.swap.parent->data.parent;

        BlockMarkNeedsUpdate(parentData->children.data[command->data.swap.firstChildI]);
        BlockMarkNeedsUpdate(parentData->children.data[command->data.swap.secondChildI]);

        BlockSwapChildren(command->data.swap.parent, command->data.swap.firstChildI, command->data.swap.secondChildI);

        break;
    }
    }

    cursor->block = command->cursorBlock;
}

static void CursorUndo(Cursor *cursor)
{
    if (cursor->commands.count < 1)
    {
        return;
    }

    Command command = ListPop_Command(&cursor->commands);

    CommandUndo(cursor, &command);
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

    if (!cursor->block->parent)
    {
        return false;
    }

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

    int32_t parentChildrenCount = BlockGetChildrenCount(cursor->block->parent);

    *childI = MathInt32Wrap(*childI, parentChildrenCount);

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

static void CursorAddChild(Cursor *cursor, Block *parent, Block *child, int32_t childI)
{
    BlockMarkNeedsUpdate(cursor->block);

    if (cursor->insertDirection == InsertDirectionCenter)
    {
        CommandReplaceChild(cursor, parent, child, childI);
    }
    else
    {
        CommandInsertChild(cursor, parent, child, childI);
    }

    cursor->block = child;
}

static void CursorEndInsert(Cursor *cursor)
{
    cursor->state = CursorStateMove;
    SearchBarReset(&cursor->searchBar);
}

static void CursorStartInsert(Cursor *cursor, InsertDirection insertDirection)
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
        CursorAddChild(cursor, parent, block, childI);
        CursorEndInsert(cursor);

        return;
    }
}

static void CursorShift(Cursor *cursor, InsertDirection shiftDirection)
{
    if (!cursor->block->parent)
    {
        return;
    }

    int32_t childI;
    if (!CursorGetChildIndexInDirection(cursor, shiftDirection, &childI))
    {
        return;
    }

    Block *otherBlock = BlockGetChild(cursor->block->parent, childI);

    BlockMarkNeedsUpdate(cursor->block);
    BlockMarkNeedsUpdate(otherBlock);

    CommandSwapChildren(cursor, cursor->block->parent, cursor->block->childI, childI);
}

static void CursorCopy(Cursor *cursor)
{
    if (!cursor->block->parent)
    {
        return;
    }

    if (cursor->clipboardBlock)
    {
        BlockDelete(cursor->clipboardBlock);
    }

    cursor->clipboardBlock = BlockCopy(cursor->block, NULL, 0);
    cursor->clipboardDefaultChildKind = BlockGetDefaultChild(cursor->block->parent, cursor->block->childI);
}

static void CursorCut(Cursor *cursor)
{
    CursorCopy(cursor);
    CursorDeleteHere(cursor);
}

static void CursorPaste(Cursor *cursor)
{
    if (!cursor->block->parent || !cursor->clipboardBlock)
    {
        return;
    }

    if (!BlockCanSwapWith(cursor->block, cursor->clipboardDefaultChildKind))
    {
        return;
    }

    Block *pastedBlock = BlockCopy(cursor->clipboardBlock, cursor->block->parent, cursor->block->childI);

    BlockMarkNeedsUpdate(cursor->block);

    CommandReplaceChild(cursor, cursor->block->parent, pastedBlock, cursor->block->childI);

    cursor->block = pastedBlock;
}

static void CursorUpdateMove(Cursor *cursor, Input *input)
{
    bool isShiftHeld = InputIsButtonHeld(input, GLFW_KEY_LEFT_SHIFT) || InputIsButtonHeld(input, GLFW_KEY_RIGHT_SHIFT);

    if (isShiftHeld)
    {
        if (InputIsButtonPressedOrRepeat(input, GLFW_KEY_E))
        {
            CursorShift(cursor, InsertDirectionUp);
        }

        if (InputIsButtonPressedOrRepeat(input, GLFW_KEY_D))
        {
            CursorShift(cursor, InsertDirectionDown);
        }

        if (InputIsButtonPressedOrRepeat(input, GLFW_KEY_S))
        {
            CursorShift(cursor, InsertDirectionLeft);
        }

        if (InputIsButtonPressedOrRepeat(input, GLFW_KEY_F))
        {
            CursorShift(cursor, InsertDirectionRight);
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
        CursorDeleteHere(cursor);
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

    if (InputIsButtonPressed(input, GLFW_KEY_X))
    {
        CursorCut(cursor);
    }

    if (InputIsButtonPressed(input, GLFW_KEY_C))
    {
        CursorCopy(cursor);
    }

    if (InputIsButtonPressed(input, GLFW_KEY_V))
    {
        CursorPaste(cursor);
    }

    if (InputIsButtonPressedOrRepeat(input, GLFW_KEY_Z))
    {
        CursorUndo(cursor);
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

static void CursorUpdateInsert(Cursor *cursor, Input *input, Font *font)
{
    int32_t childI;
    CursorGetChildInsertIndexInDirection(cursor, &childI);

    const DefaultChildKind *defaultChildKind = BlockGetDefaultChild(cursor->block->parent, childI);
    Block *parent = cursor->block->parent;

    const PinKindInsertBlocks *insertBlocks = &PinInsertBlocks[defaultChildKind->pinKind];
    SearchBarState searchState = SearchBarUpdate(&cursor->searchBar, input);

    if (searchState == SearchBarStateUpdated)
    {
        SearchBarClearSearchResults(&cursor->searchBar);

        if (cursor->searchBar.text.count != 0)
        {
            for (int32_t i = 0; i < insertBlocks->blockKindIdCount; i++)
            {
                BlockKindId kindId = insertBlocks->blockKindIds[i];
                const BlockKind *kind = &BlockKinds[kindId];

                SearchBarTryAddResult(&cursor->searchBar, kind->searchText);
            }
        }
    }

    if (searchState == SearchBarStateConfirm && parent && defaultChildKind->isPin)
    {
        if (cursor->searchBar.text.count == 0 && defaultChildKind->isPin)
        {
            Block *block = BlockNew(BlockKindIdPin, parent, childI);
            CursorAddChild(cursor, parent, block, childI);
            CursorEndInsert(cursor);

            return;
        }

        for (int32_t i = 0; i < insertBlocks->blockKindIdCount; i++)
        {
            BlockKindId kindId = insertBlocks->blockKindIds[i];
            const BlockKind *kind = &BlockKinds[kindId];

            if (!ListMatchesString(&cursor->searchBar.text, kind->searchText))
            {
                continue;
            }

            Block *block = BlockNew(kindId, parent, childI);
            CursorAddChild(cursor, parent, block, childI);
            CursorEndInsert(cursor);

            return;
        }

        if (defaultChildKind->pinKind == PinKindIdentifier || defaultChildKind->pinKind == PinKindExpression)
        {
            Block *block =
                BlockNewIdentifier(cursor->searchBar.text.data, cursor->searchBar.text.count, font, parent, childI);
            CursorAddChild(cursor, parent, block, childI);
            CursorEndInsert(cursor);

            return;
        }
    }

    if (searchState == SearchBarStateCancel)
    {
        CursorEndInsert(cursor);

        return;
    }
}

void CursorUpdate(Cursor *cursor, Input *input, Font *font)
{
    switch (cursor->state)
    {
    case CursorStateMove: {
        CursorUpdateMove(cursor, input);
        break;
    }
    case CursorStateInsert: {
        CursorUpdateInsert(cursor, input, font);
        break;
    }
    }
}

static void CursorDrawMove(Cursor *cursor, Camera *camera, Theme *theme)
{
    ColorSet(theme->cursorColor);

    DrawRect(cursor->x, cursor->y, cursor->width, LineWidth, camera->zoom);
    DrawRect(cursor->x, cursor->y + cursor->height - LineWidth, cursor->width, LineWidth, camera->zoom);
    DrawRect(cursor->x, cursor->y, LineWidth, cursor->height, camera->zoom);
    DrawRect(cursor->x + cursor->width - LineWidth, cursor->y, LineWidth, cursor->height, camera->zoom);
}

void CursorDraw(Cursor *cursor, Camera *camera, Font *font, Theme *theme, float deltaTime)
{
    Block *block = cursor->block;
    int32_t blockGlobalX = 0;
    int32_t blockGlobalY = 0;

    BlockGetGlobalPosition(block, &blockGlobalX, &blockGlobalY);

    float targetX = blockGlobalX - BlockPadding - LineWidth;
    float targetY = blockGlobalY - BlockPadding - LineWidth;
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

    switch (cursor->state)
    {
    case CursorStateMove: {
        CursorDrawMove(cursor, camera, theme);
        break;
    }
    case CursorStateInsert: {
        SearchBarDraw(&cursor->searchBar, camera, font, theme);
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

    cursor->block = BlockGetChild(cursor->block, 0);
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

    int32_t nextI = MathInt32Wrap(cursor->block->childI + delta, parentChildrenCount);

    cursor->block = BlockGetChild(cursor->block->parent, nextI);
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

void CursorDeleteHere(Cursor *cursor)
{
    Block *parent = cursor->block->parent;
    int32_t childI = cursor->block->childI;

    if (!parent)
    {
        return;
    }

    BlockDeleteResult deleteResult = CommandDeleteChild(cursor, parent, childI);
    BlockMarkNeedsUpdate(parent);

    BlockParentData *parentParentData = &parent->data.parent;

    if (deleteResult.wasRemoved)
    {
        if (childI < parentParentData->children.count)
        {
            cursor->block = parentParentData->children.data[childI];
        }
        else if (childI > 0)
        {
            cursor->block = parentParentData->children.data[childI - 1];
        }
        else
        {
            cursor->block = parent;
        }
    }
    else
    {
        cursor->block = parentParentData->children.data[childI];
    }
}