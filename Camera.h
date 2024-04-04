#pragma once

typedef struct Block Block;
typedef struct Cursor Cursor;

typedef struct Camera
{
    float x;
    float y;
    float width;
    float height;

    float zoom;
} Camera;

Camera CameraNew();
void CameraUpdate(Camera *camera, Cursor *cursor, Block *rootBlock, float deltaTime);
void CameraZoomIn(Camera *camera);
void CameraZoomOut(Camera *camera);