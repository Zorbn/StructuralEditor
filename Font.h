#pragma once

struct Font;

struct Font *FontNew(const char *path, float size);
int FontDelete(struct Font *font);
int DrawText(const char *text, float x, float y, struct Font *font);
int GetTextSize(const char *text, int *width, int *height, struct Font *font);