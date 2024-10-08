#include "Camera.h"
#include "Block.h"
#include "Cursor.h"
#include "Math.h"

const float PanSpeed = 10.0f;
const float ZoomIncrement = 0.25f;

Camera CameraNew(void)
{
    return (Camera){
        .x = 0.0f,
        .y = 0.0f,
        .width = 0.0f,
        .height = 0.0f,
        .zoom = 1.0f,
        .needsTeleport = true,
    };
}

void CameraUpdate(Camera *camera, Cursor *cursor, Block *rootBlock, float deltaTime)
{
    int32_t textWidth, textHeight;
    BlockGetTextSize(cursor->block, &textWidth, &textHeight);

    float width = camera->width / camera->zoom;
    float height = camera->height / camera->zoom;

    float targetX = -BlockPaddingX * 2.0f;
    targetX = MathFloatMin(targetX, rootBlock->x + rootBlock->width * 0.5f - width * 0.5f);

    int32_t cursorBlockGlobalX = 0;
    int32_t cursorBlockGlobalY = 0;
    BlockGetGlobalPosition(cursor->block, &cursorBlockGlobalX, &cursorBlockGlobalY);

    float targetY = cursorBlockGlobalY - height * 0.5f;

    if (cursor->block->height > height)
    {
        targetY += textHeight * 0.5f;
    }
    else
    {
        targetY += cursor->block->height * 0.5f;
    }

    if (camera->needsTeleport)
    {
        camera->x = targetX;
        camera->y = targetY;

        camera->needsTeleport = false;
    }

    const float stopDistance = 1.0f;

    float delta = PanSpeed * deltaTime;
    camera->x = MathLazyLerp(camera->x, targetX, delta, stopDistance);
    camera->y = MathLazyLerp(camera->y, targetY, delta, stopDistance);
}

void CameraZoomIn(Camera *camera)
{
    camera->zoom += ZoomIncrement;
    camera->needsTeleport = true;
}

void CameraZoomOut(Camera *camera)
{
    camera->zoom = MathFloatMax(camera->zoom - ZoomIncrement, ZoomIncrement);
    camera->needsTeleport = true;
}