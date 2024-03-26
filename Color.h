#pragma once

typedef struct Color
{
    float r;
    float g;
    float b;
    float a;
} Color;

Color ColorNew(float r, float g, float b);
Color ColorNew255(float r, float g, float b);
void ColorSet(Color color);