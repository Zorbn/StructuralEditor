#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>

#define SOKOL_GLCORE33
#include <sokol_gfx.h>
#include <sokol_gp.h>
#include <sokol_log.h>

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include "Block.h"
#include "Cursor.h"
#include "Font.h"
#include "Input.h"
#include "Parser.h"
#include "Theme.h"
#include "Camera.h"
#include "Math.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * TODO, Missing things from the other version:
 * Zooming,
 *  - On zoom, for fonts: UpdateTreeFont, UpdateTree and BlockKindsUpdateTextSize with new font size.
 * Saving,
 * Text insert display,
 *
 * TODO, New features:
 * Auto-complete for insert mode,
 * Search for text in tree,
 * Support multiple files,
 *
 * TODO, Ideas:
 * Search for patterns structurally, eg. search for a fn with the name "hello world" and a third argument named "c",
 * Support more symbols in identifiers, such as ?, so "enabled?" generates "is_enabled" or something,
 */

typedef struct WindowData
{
    Input *input;
    Camera *camera;
} WindowData;

static void WindowKeyCallback(GLFWwindow *window, int key, int scanCode, int action, int modifiers)
{
    WindowData *windowData = glfwGetWindowUserPointer(window);
    InputUpdateButton(windowData->input, key, action);
}

static void Utf32ToUtf8(List_char *destination, uint32_t codePoint)
{
    if (codePoint <= 0x7f)
    {
        ListPush_char(destination, (char)codePoint);
    }
    else if (codePoint <= 0x7ff)
    {
        ListPush_char(destination, 0xc0 | (char)(codePoint >> 6));
    }
    else if (codePoint <= 0xffff)
    {
        ListPush_char(destination, 0xe0 | (char)(codePoint >> 12));
        ListPush_char(destination, 0x80 | (char)((codePoint >> 6) & 0x3f));
        ListPush_char(destination, 0x80 | (char)(codePoint & 0x3f));
    }
    else if (codePoint <= 0x10ffff)
    {
        ListPush_char(destination, 0xf0 | (char)(codePoint >> 18));
        ListPush_char(destination, 0x80 | (char)((codePoint >> 12) & 0x3f));
        ListPush_char(destination, 0x80 | (char)((codePoint >> 6) & 0x3f));
        ListPush_char(destination, 0x80 | (char)(codePoint & 0x3f));
    }
}

static void WindowCharCallback(GLFWwindow *window, uint32_t codePoint)
{
    WindowData *windowData = glfwGetWindowUserPointer(window);
    Utf32ToUtf8(&windowData->input->typedChars, codePoint);
}

static void WindowSizeCallback(GLFWwindow *window, int32_t width, int32_t height)
{
    WindowData *windowData = glfwGetWindowUserPointer(window);
    windowData->camera->width = (float)width;
    windowData->camera->height = (float)height;
}

int main(int argumentCount, char **arguments)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, 1);
    GLFWwindow *window = glfwCreateWindow(800, 600, "Structural Editor", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    Input input = InputNew();
    Camera camera = CameraNew();

    WindowData windowData = (WindowData){
        .input = &input,
        .camera = &camera,
    };

    glfwSetWindowUserPointer(window, &windowData);
    glfwSetKeyCallback(window, WindowKeyCallback);
    glfwSetCharCallback(window, WindowCharCallback);
    glfwSetWindowSizeCallback(window, WindowSizeCallback);

    {
        int32_t width, height;
        glfwGetWindowSize(window, &width, &height);
        WindowSizeCallback(window, width, height);
    }

    sg_desc sokolGfxDescriptor = (sg_desc){
        .logger.func = slog_func,
    };
    sg_setup(&sokolGfxDescriptor);
    assert(sg_isvalid());

    sgp_desc sokolGpDescriptor = (sgp_desc){0};
    sgp_setup(&sokolGpDescriptor);
    assert(sgp_is_valid());

    char *data = NULL;
    int32_t dataCount = 0;
    {
        char *path = "save.lua";

        if (argumentCount >= 2)
        {
            path = arguments[1];
        }

        FILE *file = fopen(path, "r");
        if (!file)
        {
            printf("Couldn't open file \"%s\"", path);
            exit(EXIT_FAILURE);
        }

        fseek(file, 0, SEEK_END);
        dataCount = ftell(file);
        fseek(file, 0, SEEK_SET);

        data = malloc(dataCount + 1);

        fread(data, dataCount, 1, file);
        fclose(file);

        data[dataCount] = '\0';
    }

    Font *font = FontNew("DejaVuSans.ttf", 16);
    Theme theme = (Theme){
        .backgroundColor = ColorNew255(51, 51, 51),
        .evenColor = ColorNew255(40, 40, 40),
        .oddColor = ColorNew255(33, 33, 33),
        .textColor = ColorNew255(253, 244, 193),
        .cursorColor = ColorNew255(221, 111, 72),
        .pinColor = ColorNew255(124, 111, 100),
    };

    BlockKindsInit();
    BlockKindsUpdateTextSize(font);

    Parser parser = ParserNew(LexerNew(data, dataCount), font);
    Block *rootBlock = ParserParseStatement(&parser, NULL, 0);
    Cursor cursor = CursorNew(rootBlock);

    BlockUpdateTree(rootBlock, 0, 0);

    printf("Block count: %llu\n", BlockCountAll(rootBlock));
    printf("Block size individual: %zd\n", sizeof(Block));
    printf("Block kind size: %zd\n", sizeof(BlockKindId));

    double lastFrameTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        // Update:
        double frameTime = glfwGetTime();
        float deltaTime = (float)(frameTime - lastFrameTime);
        lastFrameTime = frameTime;

        // printf("%f\n", 1.0f / deltaTime);

        CursorUpdate(&cursor, &input, rootBlock, font);
        CameraUpdate(&camera, &cursor, rootBlock, deltaTime);
        InputUpdate(&input);

        // Draw:
        sgp_begin((int32_t)camera.width, (int32_t)camera.height);
        sgp_viewport(0, 0, (int32_t)camera.width, (int32_t)camera.height);

        sgp_translate(MathFloatFloor(-camera.x), MathFloatFloor(-camera.y));

        // sgp_set_color(0.1f, 0.1f, 0.1f, 1.0f);
        ColorSet(theme.backgroundColor);
        sgp_clear();

        // sgp_set_color(1.0f, 0.5f, 0.3f, 1.0f);
        // sgp_draw_filled_rect(0, 0, 100, 50);

        // sgp_set_color(1.0f, 1.0f, 1.0f, 1.0f);
        // DrawText("hello", 10, 0, font);
        // DrawText("abcd", 10, 0, font);
        // DrawText("efgh", 10, 0, font);
        // DrawText("ijkl", 10, 0, font);

        // BlockDraw(cursorBlock, width / 2, height / 2, 0, height, font, &drawCommands);
        BlockDraw(rootBlock, cursor.block, 0, (int32_t)camera.y, (int32_t)(camera.y + camera.height), font, &theme);
        CursorDraw(&cursor, &theme, deltaTime);

        sg_pass_action passAction = {0};
        sg_begin_default_pass(&passAction, (int32_t)camera.width, (int32_t)camera.height);
        sgp_flush();
        sgp_end();
        sg_end_pass();
        sg_commit();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    CursorDelete(&cursor);
    BlockDelete(rootBlock);
    FontDelete(font);
    ParserDelete(&parser);
    free(data);

    BlockKindsDeinit();

    InputDelete(&input);

    sgp_shutdown();
    sg_shutdown();
    glfwTerminate();

    printf("Has memory leaks?: %d\n", _CrtDumpMemoryLeaks());
}
