#include "Math.h"

#include <math.h>

int32_t MathInt32Max(int32_t a, int32_t b)
{
    int32_t aIsMax = a > b;
    return aIsMax * a + !aIsMax * b;
}

int32_t MathInt32Min(int32_t a, int32_t b)
{
    int32_t aIsMin = a < b;
    return aIsMin * a + !aIsMin * b;
}

int32_t MathInt32Clamp(int32_t value, int32_t min, int32_t max)
{
    return MathInt32Min(max, MathInt32Max(value, min));
}

float MathFloatMax(float a, float b)
{
    float aIsMax = (float)(a > b);
    return aIsMax * a + !aIsMax * b;
}

float MathFloatMin(float a, float b)
{
    float aIsMin = (float)(a < b);
    return aIsMin * a + !aIsMin * b;
}

float MathFloatAbs(float a)
{
    float aIsNegative = (float)(a < 0);
    return aIsNegative * -a + !aIsNegative * a;
}

float MathFloatFloor(float a)
{
    return (float)floor(a);
}

float MathFloatCeil(float a)
{
    return (float)ceil(a);
}

float MathLerp(float start, float end, float delta)
{
    return start + (end - start) * delta;
}

// Lerp until you're already close enough.
// Useful when you're lerping something visual, and you want to avoid artifacts like text jitter.
float MathLazyLerp(float start, float end, float delta, float stopDistance)
{
    float displacement = end - start;

    if (MathFloatAbs(displacement) < stopDistance)
    {
        return start;
    }

    return start + displacement * delta;
}