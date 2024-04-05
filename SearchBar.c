#include "SearchBar.h"
#include "Math.h"
#include "Shapes.h"

#include <sokol_gfx.h>
#include <sokol_gp.h>

SearchBar SearchBarNew(void)
{
    return (SearchBar){
        .text = ListNew_char(16),
        .results = ListNew_CharPointer(16),
    };
}

void SearchBarDelete(SearchBar *searchBar)
{
    ListDelete_char(&searchBar->text);
    ListDelete_CharPointer(&searchBar->results);
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

static void SearchBarConfirm(SearchBar *searchBar)
{
    if (searchBar->selectedI == 0)
    {
        return;
    }

    char *result = searchBar->results.data[searchBar->selectedI - 1];

    if (!result)
    {
        return;
    }

    ListReset_char(&searchBar->text);

    for (int32_t i = 0; result[i] != '\0'; i++)
    {
        ListPush_char(&searchBar->text, result[i]);
    }
}

SearchBarState SearchBarUpdate(SearchBar *searchBar, Input *input)
{
    bool isCanceling = InputIsButtonPressed(input, GLFW_KEY_ESCAPE);
    bool isConfirming = InputIsButtonPressed(input, GLFW_KEY_ENTER);
    bool isControlHeld =
        InputIsButtonHeld(input, GLFW_KEY_LEFT_CONTROL) || InputIsButtonHeld(input, GLFW_KEY_RIGHT_CONTROL);
    bool isShiftHeld =
        InputIsButtonHeld(input, GLFW_KEY_LEFT_SHIFT) || InputIsButtonHeld(input, GLFW_KEY_RIGHT_SHIFT);

    bool wasUpdated = false;

    for (int32_t i = 0; i < input->typedChars.count; i++)
    {
        ListPush_char(&searchBar->text, input->typedChars.data[i]);
        wasUpdated = true;
    }

    if (InputIsButtonPressedOrRepeat(input, GLFW_KEY_BACKSPACE))
    {
        SearchBarBackspace(searchBar, isControlHeld);
        wasUpdated = true;
    }

    if (InputIsButtonPressedOrRepeat(input, GLFW_KEY_TAB))
    {
        if (isShiftHeld)
        {
            searchBar->selectedI -= 1;
        }
        else
        {
            searchBar->selectedI += 1;
        }

        searchBar->selectedI = MathInt32Wrap(searchBar->selectedI, searchBar->results.count + 1);
    }

    if (isCanceling)
    {
        return SearchBarStateCancel;
    }

    if (isConfirming)
    {
        SearchBarConfirm(searchBar);

        return SearchBarStateConfirm;
    }

    return wasUpdated ? SearchBarStateUpdated : SearchBarStateIdle;
}

void SearchBarReset(SearchBar *searchBar)
{
    ListReset_char(&searchBar->text);
    SearchBarClearSearchResults(searchBar);
}

static bool SearchBarPartialMatches(SearchBar *searchBar, char *string)
{
    if (!string)
    {
        return false;
    }

    for (int32_t i = 0; i < searchBar->text.count; i++)
    {
        if (string[i] == '\0' || searchBar->text.data[i] != string[i])
        {
            return false;
        }
    }

    return true;
}

bool SearchBarTryAddResult(SearchBar *searchBar, char *result)
{
    if (SearchBarPartialMatches(searchBar, result))
    {
        ListPush_CharPointer(&searchBar->results, result);
        return true;
    }

    return false;
}

void SearchBarClearSearchResults(SearchBar *searchBar)
{
    ListReset_CharPointer(&searchBar->results);
    searchBar->selectedI = 0;
}

void SearchBarDraw(SearchBar *searchBar, Camera *camera, Font *font, Theme *theme)
{
    sgp_push_transform();
    sgp_translate(MathFloatFloor(camera->x * camera->zoom + camera->width * 0.5f),
        MathFloatFloor(camera->y * camera->zoom + camera->height * 0.5f));

    ListPush_char(&searchBar->text, '\0');

    int32_t iWidth = 0;
    int32_t iHeight = 0;
    int32_t iDescent = 0;
    FontGetTextSize(searchBar->text.data, &iWidth, &iHeight, NULL, &iDescent, font);

    float width = (float)iWidth / camera->zoom;
    float height = (float)iHeight / camera->zoom;
    float descent = (float)iDescent / camera->zoom;

    float x = -width;
    float y = -height * 0.5f;

    float backgroundX = x - BlockPadding;
    float backgroundY = y - descent - BlockPadding;
    float backgroundWidth = width + BlockPadding * 2.0f;
    float backgroundHeight = height + BlockPadding * 2.0f;

    if (searchBar->selectedI == 0)
    {
        ColorSet(theme->cursorColor);
        DrawRect(backgroundX - LineWidth, backgroundY - LineWidth, backgroundWidth + LineWidth * 2.0f,
            backgroundHeight + LineWidth * 2.0f, camera->zoom);
    }

    ColorSet(theme->oddColor);
    DrawRect(backgroundX, backgroundY, backgroundWidth, backgroundHeight, camera->zoom);

    ColorSet(theme->textColor);
    FontDraw(searchBar->text.data, MathFloatFloor(x * camera->zoom), MathFloatFloor(y * camera->zoom), font);

    ListPop_char(&searchBar->text);

    y += height;

    for (int32_t i = 0; i < searchBar->results.count; i++)
    {
        y += BlockPadding * 3;

        char *result = searchBar->results.data[i];

        int32_t resultIWidth = 0;
        int32_t resultIHeight = 0;
        int32_t resultIDescent = 0;
        FontGetTextSize(result, &resultIWidth, &resultIHeight, NULL, &resultIDescent, font);

        float resultWidth = (float)resultIWidth / camera->zoom;
        float resultHeight = (float)resultIHeight / camera->zoom;
        float resultDescent = (float)resultIDescent / camera->zoom;

        float resultBackgroundX = x - BlockPadding;
        float resultBackgroundY = y - resultDescent - BlockPadding;
        float resultBackgroundWidth = resultWidth + BlockPadding * 2.0f;
        float resultBackgroundHeight = resultHeight + BlockPadding * 2.0f;

        if (searchBar->selectedI - 1 == i)
        {
            ColorSet(theme->cursorColor);
            DrawRect(resultBackgroundX - LineWidth, resultBackgroundY - LineWidth, resultBackgroundWidth + LineWidth * 2.0f,
                resultBackgroundHeight + LineWidth * 2.0f, camera->zoom);
        }

        ColorSet(theme->oddColor);
        DrawRect(resultBackgroundX, resultBackgroundY, resultBackgroundWidth, resultBackgroundHeight, camera->zoom);

        ColorSet(theme->textColor);
        FontDraw(result, MathFloatFloor(x * camera->zoom), MathFloatFloor(y * camera->zoom), font);

        y += resultHeight;
    }

    // ColorSet(theme->cursorColor);
    // DrawRect(centerX, y - descent, LineWidth, (float)height, camera->zoom);

    sgp_pop_transform();
}