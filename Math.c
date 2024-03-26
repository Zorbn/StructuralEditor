#include "Math.h"

int32_t Int32Max(int32_t a, int32_t b)
{
    int32_t aIsMax = a > b;
    return aIsMax * a + !aIsMax * b;
}

int32_t Int32Min(int32_t a, int32_t b)
{
    int32_t aIsMin = a < b;
    return aIsMin * a + !aIsMin * b;
}