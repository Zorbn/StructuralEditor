#include "Shapes.h"
#include "Math.h"

#include <sokol_gfx.h>
#include <sokol_gp.h>

#include <stdio.h>

void DrawRect(float x, float y, float width, float height, float scale)
{
    sgp_draw_filled_rect(x * scale, y * scale, width * scale, height * scale);
}

void DrawRectBordered(float x, float y, float width, float height, float scale, float outline)
{
    outline = MathFloatCeil(outline * scale);
    sgp_draw_filled_rect(x * scale - outline, y * scale - outline, width * scale + outline * 2, height * scale + outline * 2);
}

void RectangleDraw(Rectangle *rectangle, float scale)
{
    DrawRect(rectangle->x, rectangle->y, rectangle->width, rectangle->height, scale);
}