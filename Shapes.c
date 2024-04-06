#include "Shapes.h"
#include "Math.h"

#include <sokol_gfx.h>
#include <sokol_gp.h>

void DrawRect(float x, float y, float width, float height, float scale)
{
    sgp_draw_filled_rect(x * scale, y * scale, width * scale, height * scale);
}

void RectangleDraw(Rectangle *rectangle, float scale)
{
    DrawRect(rectangle->x, rectangle->y, rectangle->width, rectangle->height, scale);
}