#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define SOKOL_GLCORE33
#include <sokol_gfx.h>
#include <sokol_log.h>
#include <sokol_gp.h>

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include "Font.h"
#include "Parser.h"
#include "Block.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
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
        FILE *file = fopen("save.lua", "r");

        fseek(file, 0, SEEK_END);
        dataCount = ftell(file);
        fseek(file, 0, SEEK_SET);

        data = malloc(dataCount + 1);

        fread(data, dataCount, 1, file);
        fclose(file);

        data[dataCount] = '\0';
    }

    Font *font = FontNew("DejaVuSans.ttf", 16);

    BlockKindsInit(font);

    Parser parser = ParserNew(LexerNew(data, dataCount));
    Block *rootBlock = ParserParseStatement(&parser, NULL, 0);
    Block *cursorBlock = rootBlock->children[4]->children[1]->children[1];

    printf("Block count: %llu\n", BlockCountAll(rootBlock));
    printf("Block size all: %llu\n", BlockSizeAll(rootBlock));
    printf("Block size individual: %zd\n", sizeof(Block));
    printf("Block kind size: %zd\n", sizeof(BlockKindId));

    List_DrawCommand drawCommands = ListNew_DrawCommand(64);

    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        sgp_begin(width, height);
        sgp_viewport(0, 0, width, height);

        sgp_set_color(0.1f, 0.1f, 0.1f, 1.0f);
        sgp_clear();

        sgp_set_color(1.0f, 0.5f, 0.3f, 1.0f);
        sgp_draw_filled_rect(0, 0, 100, 50);

        sgp_set_color(1.0f, 1.0f, 1.0f, 1.0f);
        // DrawText("hello", 10, 0, font);
        // DrawText("abcd", 10, 0, font);
        // DrawText("efgh", 10, 0, font);
        // DrawText("ijkl", 10, 0, font);

        ListReset_DrawCommand(&drawCommands);
        // BlockDraw(cursorBlock, width / 2, height / 2, 0, height, font, &drawCommands);
        BlockDraw(rootBlock, 600, 0, 0, height, font, &drawCommands);
        DrawCommandHandle(&drawCommands, font);

        sg_pass_action passAction = {0};
        sg_begin_default_pass(&passAction, width, height);
        sgp_flush();
        sgp_end();
        sg_end_pass();
        sg_commit();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    BlockDelete(rootBlock);
    FontDelete(font);
    free(data);

    BlockKindsDeinit();
    ListDelete_DrawCommand(&drawCommands);

    sgp_shutdown();
    sg_shutdown();
    glfwTerminate();

    printf("Has memory leaks?: %d\n", _CrtDumpMemoryLeaks());
}
