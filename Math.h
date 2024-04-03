#pragma once

#include <inttypes.h>

int32_t MathInt32Max(int32_t a, int32_t b);
int32_t MathInt32Min(int32_t a, int32_t b);
int32_t MathInt32Clamp(int32_t value, int32_t min, int32_t max);
float MathFloatMin(float a, float b);
float MathFloatMax(float a, float b);
float MathFloatClamp(float value, float min, float max);
float MathFloatAbs(float a);
float MathFloatFloor(float a);
float MathFloatCeil(float a);
float MathLerp(float start, float end, float delta);
float MathLazyLerp(float start, float end, float delta, float stopDistance);