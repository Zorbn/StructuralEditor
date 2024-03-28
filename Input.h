#pragma once

#include "List.h"

#include <GLFW/glfw3.h>

#include <inttypes.h>
#include <stdbool.h>

ListDefine(int32_t);

typedef struct Input
{
    struct List_int32_t heldButtons;
    struct List_int32_t pressedButtons;
    struct List_int32_t repeatButtons;
} Input;

Input InputNew(void);
void InputUpdateButton(Input *input, int32_t button, int32_t action);
void InputUpdate(Input *input);
bool InputIsButtonHeld(Input *input, int32_t button);
bool InputIsButtonPressed(Input *input, int32_t button);
bool InputIsButtonRepeat(Input *input, int32_t button);
bool InputIsButtonPressedOrRepeat(Input *input, int32_t button);
void InputDelete(Input *input);