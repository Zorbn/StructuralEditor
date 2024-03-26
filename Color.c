#include "Color.h"

#define SOKOL_GLCORE33
#include <sokol_gfx.h>
#include <sokol_gp.h>

Color ColorNew(float r, float g, float b)
{
    return (Color){
        .r = r,
        .g = g,
        .b = b,
        .a = 1.0f,
    };
}

Color ColorNew255(float r, float g, float b)
{
    return ColorNew(r / 255.0f, g / 255.0f, b / 255.0f);
}

void ColorSet(Color color)
{
    sgp_set_color(color.r, color.g, color.b, color.a);
}