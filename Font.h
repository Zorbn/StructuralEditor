#pragma once

typedef struct Font Font;

Font *FontNew(const char *path, float size);
int FontDelete(Font *font);
int DrawText(const char *text, float x, float y, Font *font);
int GetTextSize(const char *text, int *width, int *height, Font *font);