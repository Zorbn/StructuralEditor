#pragma once

#include "List.h"

#include <GLFW/glfw3.h>

#include <inttypes.h>
#include <stdbool.h>

ListDefine(int32_t);
ListDefine(char);

typedef struct Input
{
    List_int32_t heldButtons;
    List_int32_t pressedButtons;
    List_int32_t repeatButtons;
    List_char typedChars;
} Input;

Input InputNew(void);
void InputUpdateButton(Input *input, int32_t button, int32_t action);
void InputUpdate(Input *input);
bool InputIsButtonHeld(Input *input, int32_t button);
bool InputIsButtonPressed(Input *input, int32_t button);
bool InputIsButtonRepeat(Input *input, int32_t button);
bool InputIsButtonPressedOrRepeat(Input *input, int32_t button);
void InputDelete(Input *input);