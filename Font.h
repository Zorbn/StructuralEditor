#pragma once

#include <inttypes.h>

typedef struct Font Font;

Font *FontNew(const char *path, float size);
int FontDelete(Font *font);
void FontUpdate(Font *font);
int FontDraw(const char *text, float x, float y, Font *font);
int FontGetTextSize(const char *text, int *width, int *height, Font *font);