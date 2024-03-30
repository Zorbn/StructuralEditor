#include "Geometry.h"

float GeometryLerp(float start, float end, float delta)
{
    return start + (end - start) * delta;
}