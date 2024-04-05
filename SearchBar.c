#include "SearchBar.h"
#include "Math.h"
#include "Shapes.h"

#include <sokol_gfx.h>
#include <sokol_gp.h>

SearchBar SearchBarNew(void)
{
    return (SearchBar){
        .text = ListNew_char(16),
    };
}

void SearchBarDelete(SearchBar *searchBar)
{
    ListDelete_char(&searchBar->text);
}

static void SearchBarBackspace(SearchBar *searchBar, bool isControlHeld)
{
    if (searchBar->text.count < 1)
    {
        return;
    }

    if (isControlHeld)
    {
        bool reachedWhitespace = false;

        while (searchBar->text.count > 0)
        {
            char lastChar = searchBar->text.data[searchBar->text.count - 1];

            if (lastChar == ' ')
            {
                reachedWhitespace = true;
            }
            else if (reachedWhitespace)
            {
                break;
            }

            ListPop_char(&searchBar->text);
        }
    }
    else
    {
        ListPop_char(&searchBar->text);
    }
}

SearchBarState SearchBarUpdate(SearchBar *searchBar, Input *input)
{
    bool isCanceling = InputIsButtonPressed(input, GLFW_KEY_ESCAPE);
    bool isConfirming = InputIsButtonPressed(input, GLFW_KEY_ENTER);
    bool isControlHeld =
        InputIsButtonHeld(input, GLFW_KEY_LEFT_CONTROL) || InputIsButtonHeld(input, GLFW_KEY_RIGHT_CONTROL);

    for (int32_t i = 0; i < input->typedChars.count; i++)
    {
        ListPush_char(&searchBar->text, input->typedChars.data[i]);
    }

    if (InputIsButtonPressedOrRepeat(input, GLFW_KEY_BACKSPACE))
    {
        SearchBarBackspace(searchBar, isControlHeld);
    }

    if (isCanceling)
    {
        return SearchBarStateCancel;
    }

    if (isConfirming)
    {
        return SearchBarStateConfirm;
    }

    return SearchBarStateInProgress;
}

void SearchBarReset(SearchBar *searchBar)
{
    ListReset_char(&searchBar->text);
}

void SearchBarDraw(SearchBar *searchBar, Camera *camera, Font *font, Theme *theme)
{
    sgp_push_transform();
    sgp_translate(MathFloatFloor(camera->width * 0.5f), MathFloatFloor(camera->height * 0.5f));

    ListPush_char(&searchBar->text, '\0');

    int32_t iWidth = 0;
    int32_t iHeight = 0;
    int32_t iDescent = 0;
    FontGetTextSize(searchBar->text.data, &iWidth, &iHeight, NULL, &iDescent, font);

    float width = (float)iWidth / camera->zoom;
    float height = (float)iHeight / camera->zoom;
    float descent = (float)iDescent / camera->zoom;

    float centerX = camera->x;
    float centerY = camera->y;

    float x = centerX - width;
    float y = centerY - height * 0.5f;

    float backgroundX = x - BlockPadding;
    float backgroundY = y - descent - BlockPadding;
    float backgroundWidth = width + BlockPadding * 2.0f;
    float backgroundHeight = height + BlockPadding * 2.0f;

    ColorSet(theme->cursorColor);
    DrawRect(backgroundX - LineWidth, backgroundY - LineWidth, backgroundWidth + LineWidth * 2.0f,
        backgroundHeight + LineWidth * 2.0f, camera->zoom);
    ColorSet(theme->oddColor);
    DrawRect(backgroundX, backgroundY, backgroundWidth, backgroundHeight, camera->zoom);

    ColorSet(theme->textColor);
    FontDraw(searchBar->text.data, MathFloatFloor(x * camera->zoom), MathFloatFloor(y * camera->zoom), font);

    ListPop_char(&searchBar->text);

    // ColorSet(theme->cursorColor);
    // DrawRect(centerX, y - descent, LineWidth, (float)height, camera->zoom);

    sgp_pop_transform();
}