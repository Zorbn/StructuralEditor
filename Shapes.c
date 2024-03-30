#include "Shapes.h"

#define SOKOL_GLCORE33
#include <sokol_gfx.h>
#include <sokol_gp.h>

void DrawRect(float x, float y, float width, float height)
{
    sgp_draw_filled_rect(x, y, width, height);
}