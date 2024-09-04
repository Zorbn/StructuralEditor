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

extern const int32_t BlockPaddingX;
extern const int32_t BlockPaddingY;
extern const int32_t BlockSpacing;
extern const int32_t BlockIndent;
extern const float LineWidth;
extern const float BorderWidth;