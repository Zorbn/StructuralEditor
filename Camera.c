#include "Camera.h"
#include "Math.h"
#include "Block.h"
#include "Cursor.h"

const float PanSpeed = 10.0f;
const float ZoomIncrement = 0.25f;

Camera CameraNew(float windowWidth, float windowHeight)
{
    return (Camera){
        .x = 0.0f,
        .y = 0.0f,
        .width = 0.0f,
        .height = 0.0f,
        .zoom = 1.0f,
    };
}

void CameraUpdate(Camera *camera, Cursor *cursor, Block *rootBlock, float deltaTime)
{
    int32_t textWidth, textHeight;
    BlockGetTextSize(cursor->block, &textWidth, &textHeight);

    float width = camera->width / camera->zoom;
    float height = camera->height / camera->zoom;

    float targetX = -BlockPadding * 2.0f;
    targetX = MathFloatMin(targetX, rootBlock->x + rootBlock->width * 0.5f - width * 0.5f);

    float targetY = cursor->block->y - height * 0.5f;

    if (cursor->block->height > height)
    {
        targetY += textHeight * 0.5f;
    }
    else
    {
        targetY += cursor->block->height * 0.5f;
    }

    const float stopDistance = 1.0f;

    float delta = PanSpeed * deltaTime;
    camera->x = MathLazyLerp(camera->x, targetX, delta, stopDistance);
    camera->y = MathLazyLerp(camera->y, targetY, delta, stopDistance);
}

void CameraZoomIn(Camera *camera)
{
    camera->zoom += ZoomIncrement;
}

void CameraZoomOut(Camera *camera)
{
    camera->zoom = MathFloatMax(camera->zoom - ZoomIncrement, ZoomIncrement);
}