#pragma once

#include "inttypes.h"

typedef struct Rectangle
{
    float x;
    float y;
    float width;
    float height;
} Rectangle;

void DrawRect(float x, float y, float width, float height, float scale);
void DrawRectBordered(float x, float y, float width, float height, float scale, float outline);
void RectangleDraw(Rectangle *rectangle, float scale);