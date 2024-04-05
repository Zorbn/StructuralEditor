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

typedef enum CursorState
{
    CursorStateMove,
    CursorStateInsert,
} CursorState;

typedef struct Cursor
{
    SearchBar searchBar;

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