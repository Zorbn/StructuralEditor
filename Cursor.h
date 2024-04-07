#pragma once

#include "Block.h"
#include "Input.h"
#include "Camera.h"
#include "SearchBar.h"

typedef enum InsertDirection
{
    InsertDirectionUp,
    InsertDirectionDown,
    InsertDirectionLeft,
    InsertDirectionRight,
    InsertDirectionCenter,
} InsertDirection;

typedef enum CommandKind
{
    CommandKindInsert,
    CommandKindReplace,
    CommandKindDelete,
} CommandKind;

typedef struct CommandInsertData
{
    Block *parent;
    int32_t childI;
} CommandInsertData;

typedef struct CommandReplaceData
{
    Block *parent;
    Block *oldChild;
    int32_t childI;
} CommandReplaceData;

typedef struct CommandDeleteData
{
    Block *parent;
    Block *oldChild;
    int32_t childI;
    bool wasRemoved;
} CommandDeleteData;

typedef union CommandData
{
    CommandInsertData insert;
    CommandReplaceData replace;
    CommandDeleteData delete;
} CommandData;

typedef struct Command
{
    CommandData data;
    CommandKind kind;
} Command;

ListDefine(Command);

typedef enum CursorState
{
    CursorStateMove,
    CursorStateInsert,
} CursorState;

typedef struct Cursor
{
    SearchBar searchBar;
    List_Command commands;

    Block *block;
    Block *clipboardBlock;
    DefaultChildKind *clipboardDefaultChildKind;

    InsertDirection insertDirection;

    CursorState state;

    float x;
    float y;
    float width;
    float height;

    bool isFirstDraw;
} Cursor;

/*

TODO: Add Commanding for:
Delete
Cut
Paste
Shift < This one needs to be changed, it accesses block internals directly right now.
Insert

All updates to the tree are done with:
void BlockReplaceChild(Block *block, Block *child, int32_t childI);
void BlockInsertChild(Block *block, Block *child, int32_t childI);

except for a special case in CursorDeleteHere which manually calls BlockDelete and ListRemove_BlockPointer to fully remove uneeded nodes.

Maybe being able to Command these simple operations is all that is necessary to implement Command/redo for everything?

Maybe add CursorReplaceChild, CursorInsertChild, and CursorDeleteChild that all add to the Command queue, and can be Commandne/redone, then use those for all cursor operations.
Maybe it should be able to add checkpoints to the Command queue, and Commanding/redoing go to these checkpoints rather than one step at a time, since a logical operation may be composed of multiple steps.

*/

Cursor CursorNew(Block *block);
void CursorDelete(Cursor *cursor);
void CursorUpdate(Cursor *cursor, Input *input, Font *font);
void CursorDraw(Cursor *cursor, Camera *camera, Font *font, Theme *theme, float deltaTime);
void CursorAscend(Cursor *cursor);
void CursorDescend(Cursor *cursor);
void CursorNext(Cursor *cursor);
void CursorPrevious(Cursor *cursor);
void CursorUp(Cursor *cursor);
void CursorDown(Cursor *cursor);
void CursorLeft(Cursor *cursor);
void CursorRight(Cursor *cursor);
void CursorDeleteHere(Cursor *cursor);