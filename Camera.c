#include "Camera.h"
#include "Cursor.h"
#include "Math.h"

const float PanSpeed = 10.0f;

Camera CameraNew(float windowWidth, float windowHeight)
{
    return (Camera){
        .x = 0.0f,
        .y = 0.0f,
        .width = 0.0f,
        .height = 0.0f,
    };
}

void CameraUpdate(Camera *camera, Cursor *cursor, Block *rootBlock, float deltaTime)
{
    int32_t textWidth, textHeight;
    BlockGetTextSize(cursor->block, &textWidth, &textHeight);

    float targetX = -BlockPadding * 2.0f;
    targetX = MathFloatMin(targetX, rootBlock->x + rootBlock->width * 0.5f - camera->width * 0.5f);

    float targetY = cursor->block->y - camera->height * 0.5f;

    if (cursor->block->height > camera->height)
    {
        targetY += textHeight * 0.5f;
    }
    else
    {
        targetY += cursor->block->height * 0.5f;
    }

    // Stop once the camera is close enough, otherwise lerping would end with the camera moving tiny
    // subpixel amounts each frame, which would be mostly unoticable except they cause text to jitter
    // as it tries to snap to screen pixels.
    // TODO: After adding zoom this should be MathFloatCeil(camera->zoom);
    const float stopDistance = 1.0f;

    float delta = PanSpeed * deltaTime;
    camera->x = MathLazyLerp(camera->x, targetX, delta, stopDistance);
    camera->y = MathLazyLerp(camera->y, targetY, delta, stopDistance);
}