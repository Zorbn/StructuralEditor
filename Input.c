#include "Input.h"

Input InputNew(void)
{
    return (Input){
        .heldButtons = ListNew_int32_t(16),
        .pressedButtons = ListNew_int32_t(16),
        .repeatButtons = ListNew_int32_t(16),
        .typedChars = ListNew_char(16),
    };
}

static bool InputListContains(List_int32_t *buttons, int32_t button)
{
    for (size_t i = 0; i < buttons->count; i++)
    {
        if (buttons->data[i] == button)
        {
            return true;
        }
    }

    return false;
}

void InputUpdateButton(Input *input, int32_t button, int32_t action)
{
    switch (action)
    {
    case GLFW_PRESS: {
        bool wasButtonHeld = InputListContains(&input->heldButtons, button);

        if (!wasButtonHeld)
        {
            ListPush_int32_t(&input->pressedButtons, button);
        }

        ListPush_int32_t(&input->heldButtons, button);

        break;
    }
    case GLFW_RELEASE: {
        for (int32_t i = 0; i < input->heldButtons.count; i++)
        {
            if (input->heldButtons.data[i] != button)
            {
                continue;
            }

            ListRemoveUnordered_int32_t(&input->heldButtons, i);

            break;
        }

        break;
    }
    case GLFW_REPEAT: {
        ListPush_int32_t(&input->repeatButtons, button);
        break;
    }
    default: {
        break;
    }
    }
}

void InputUpdate(Input *input)
{
    ListReset_int32_t(&input->pressedButtons);
    ListReset_int32_t(&input->repeatButtons);
    ListReset_char(&input->typedChars);
}

bool InputIsButtonHeld(Input *input, int32_t button)
{
    return InputListContains(&input->heldButtons, button);
}

bool InputIsButtonPressed(Input *input, int32_t button)
{
    return InputListContains(&input->pressedButtons, button);
}

bool InputIsButtonRepeat(Input *input, int32_t button)
{
    return InputListContains(&input->repeatButtons, button);
}

bool InputIsButtonPressedOrRepeat(Input *input, int32_t button)
{
    return InputListContains(&input->pressedButtons, button) || InputListContains(&input->repeatButtons, button);
}

void InputDelete(Input *input)
{
    ListDelete_int32_t(&input->heldButtons);
    ListDelete_int32_t(&input->pressedButtons);
    ListDelete_int32_t(&input->repeatButtons);
    ListDelete_char(&input->typedChars);
}