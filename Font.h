#pragma once

#include <inttypes.h>

typedef struct Font Font;

Font *FontNew(const char *path, float size);
int FontDelete(Font *font);
int DrawText(const char *text, int32_t x, int32_t y, Font *font);
int GetTextSize(const char *text, int *width, int *height, Font *font);