#pragma once

#include <inttypes.h>

typedef struct Font Font;

Font *FontNew(const char *path, float size);
int FontDelete(Font *font);
void FontUpdate(Font *font);
int FontDraw(const char *text, float x, float y, Font *font);
int32_t FontGetTextSize(
    const char *text, int32_t *width, int32_t *height, int32_t *ascent, int32_t *descent, Font *font);