#pragma once

#include "Color.h"

#include <inttypes.h>

typedef struct Theme
{
    Color backgroundColor;
    Color borderColor;
    Color evenColor;
    Color oddColor;
    Color textColor;
    Color cursorColor;
    Color pinColor;
} Theme;

extern const int32_t BlockPadding;
extern const float LineWidth;