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

static void SearchBarGetResultSize(char *text, Camera *camera, Font *font, Rectangle *background, float *textX, float *textY)
{
    int32_t iWidth = 0;
    int32_t iHeight = 0;
    int32_t iDescent = 0;
    FontGetTextSize(text, &iWidth, &iHeight, NULL, &iDescent, font);

    float width = (float)iWidth / camera->zoom;
    float height = (float)iHeight / camera->zoom;
    float descent = (float)iDescent / camera->zoom;

    *background = (Rectangle){
        .x = -width - BlockPadding,
        .y = (float)-BlockPadding,
        .width = width + BlockPadding * 2,
        .height = height + BlockPadding * 2,
    };

    if (textX)
    {
        *textX = background->x + BlockPadding;
    }

    if (textY)
    {
        *textY = background->y + descent + BlockPadding;
    }
}

static void SearchBarGetSize(SearchBar *searchBar, Camera *camera, Font *font, Rectangle *background)
{
    ListPush_char(&searchBar->text, '\0');

    SearchBarGetResultSize(searchBar->text.data, camera, font, background, NULL, NULL);

    ListPop_char(&searchBar->text);

    float x = background->x;
    float y = background->y;
    float width = background->width;
    float height = background->height;

    for (int32_t i = 0; i < searchBar->results.count; i++)
    {
        char *result = searchBar->results.data[i];

        SearchBarGetResultSize(result, camera, font, background, NULL, NULL);

        width = MathFloatMax(width, background->width);
        height += BlockPadding + background->height;
    }

    *background = (Rectangle){
        .x = x - BlockPadding,
        .y = y - BlockPadding,
        .width = width + BlockPadding * 2,
        .height = height + BlockPadding * 2,
    };
}

void SearchBarDraw(SearchBar *searchBar, Camera *camera, Font *font, Theme *theme)
{
    sgp_push_transform();
    sgp_translate(MathFloatFloor(camera->x * camera->zoom + camera->width * 0.5f),
        MathFloatFloor(camera->y * camera->zoom + camera->height * 0.5f));

    // TODO: Use rectangle draw.

    Rectangle background = {0};
    SearchBarGetSize(searchBar, camera, font, &background);

    ColorSet(theme->borderColor);
    DrawRect(background.x - 1, background.y - 1, background.width + 2, background.height + 2, camera->zoom);

    ColorSet(theme->evenColor);
    DrawRect(background.x, background.y, background.width, background.height, camera->zoom);

    ListPush_char(&searchBar->text, '\0');

    float textX = 0;
    float textY = 0;
    SearchBarGetResultSize(searchBar->text.data, camera, font, &background, &textX, &textY);

    if (searchBar->selectedI == 0)
    {
        ColorSet(theme->cursorColor);
        DrawRect(background.x - LineWidth, background.y - LineWidth, background.width + LineWidth * 2.0f,
            background.height + LineWidth * 2.0f, camera->zoom);
    }

    ColorSet(theme->oddColor);
    DrawRect(background.x, background.y, background.width, background.height, camera->zoom);

    ColorSet(theme->textColor);
    FontDraw(searchBar->text.data, MathFloatFloor(textX * camera->zoom), MathFloatFloor(textY * camera->zoom), font);

    ListPop_char(&searchBar->text);

    float y = background.y;

    y += background.height + BlockPadding;

    for (int32_t i = 0; i < searchBar->results.count; i++)
    {
        char *result = searchBar->results.data[i];

        y += BlockPadding;

        Rectangle resultBackground = {0};
        float resultTextY = 0;
        SearchBarGetResultSize(result, camera, font, &resultBackground, NULL, &resultTextY);
        resultBackground.y += y;
        resultTextY += y;

        if (searchBar->selectedI - 1 == i)
        {
            ColorSet(theme->cursorColor);
            DrawRect(background.x - LineWidth, resultBackground.y - LineWidth, resultBackground.width + LineWidth * 2.0f,
                resultBackground.height + LineWidth * 2.0f, camera->zoom);
        }

        ColorSet(theme->oddColor);
        DrawRect(background.x, resultBackground.y, resultBackground.width, resultBackground.height, camera->zoom);

        ColorSet(theme->textColor);
        FontDraw(result, MathFloatFloor(textX * camera->zoom), MathFloatFloor(resultTextY * camera->zoom), font);

        y += resultBackground.height;
    }

    // ColorSet(theme->cursorColor);
    // DrawRect(centerX, y - descent, LineWidth, (float)height, camera->zoom);

    sgp_pop_transform();
}