#pragma once

#include "Block.h"
#include "Cursor.h"

typedef struct Camera
{
    float x;
    float y;
    float width;
    float height;
} Camera;

Camera CameraNew();
void CameraUpdate(Camera *camera, Cursor *cursor, Block *rootBlock, float deltaTime);