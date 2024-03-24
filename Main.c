#include "Font.h"

#define SOKOL_GLCORE33
#include <sokol_gfx.h>
#include <sokol_log.h>

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include "sokol_gp.h"

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

    struct Font *font = FontNew("DejaVuSans.ttf", 16);

    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float ratio = width / (float)height;

        sgp_begin(width, height);
        sgp_viewport(0, 0, width, height);

        sgp_set_color(0.1f, 0.1f, 0.1f, 1.0f);
        sgp_clear();

        sgp_set_color(1.0f, 0.5f, 0.3f, 1.0f);
        sgp_draw_filled_rect(0, 0, 100, 50);

        sgp_set_color(1.0f, 1.0f, 1.0f, 1.0f);
        DrawText("hello", 10, 0, font);

        sg_pass_action passAction = {0};
        sg_begin_default_pass(&passAction, width, height);
        sgp_flush();
        sgp_end();
        sg_end_pass();
        sg_commit();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    FontDelete(font);

    sgp_shutdown();
    sg_shutdown();
    glfwTerminate();
}
