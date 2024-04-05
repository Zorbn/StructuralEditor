#include "Font.h"

#include <sokol_gfx.h>
#include <sokol_log.h>

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include "sokol_gp.h"

#include "fontstash.h"

#ifndef FONT_ATLAS_SIZE
#define FONT_ATLAS_SIZE 1024
#endif

#ifndef INIT_NUM_FONTITEMS
#define INIT_NUM_FONTITEMS 10
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int32_t fonsAddFontMem(FONScontext *stash, const char *name, unsigned char *data, int32_t dataSize, int32_t freeData);

// Font handling ported from Lyte2D.
typedef struct Font
{
    FONScontext *context;
    void *data;
    uint8_t *buffer;
    sg_image image;
    sg_sampler sampler;
    int32_t width;
    int32_t height;
    int32_t id;
    int32_t atlasDimensions;
    float size;
    bool needsUpdate;
} Font;

static int32_t FontStashRenderCreate(void *user, int32_t width, int32_t height)
{
    Font *font = (Font *)user;
    font->width = width;
    font->height = height;

    sg_image_desc imageDescriptor = (sg_image_desc){
        .width = width,
        .height = height,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .type = SG_IMAGETYPE_2D,
        .usage = SG_USAGE_DYNAMIC,
    };

    sg_sampler_desc samplerDescriptor = (sg_sampler_desc){
        .min_filter = SG_FILTER_LINEAR,
        .mag_filter = SG_FILTER_LINEAR,
    };

    font->image = sg_make_image(&imageDescriptor);
    font->sampler = sg_make_sampler(&samplerDescriptor);
    font->buffer = malloc(width * height * 4);
    return 1;
}

static void FontStashRenderDelete(void *user)
{
    Font *font = (Font *)user;
    sg_destroy_image(font->image);
    sg_destroy_sampler(font->sampler);
    font->image = (sg_image){0};
    free(font->buffer);
}

static int32_t FontStashRenderResize(void *user, int32_t width, int32_t height)
{
    FontStashRenderDelete(user);

    return FontStashRenderCreate(user, width, height);
}

static void FontStashRenderUpdate(void *user, int32_t *rectangle, const uint8_t *data)
{
    Font *font = (Font *)user;

    int32_t x1 = rectangle[0];
    int32_t y1 = rectangle[1];
    int32_t x2 = rectangle[2];
    int32_t y2 = rectangle[3];

    for (int32_t iy = y1; iy < y2; iy++)
    {
        for (int32_t ix = x1; ix < x2; ix++)
        {
            int32_t i = ix + iy * font->width;
            font->buffer[i * 4] = (uint8_t)255;
            font->buffer[i * 4 + 1] = (uint8_t)255;
            font->buffer[i * 4 + 2] = (uint8_t)255;
            font->buffer[i * 4 + 3] = data[i];
        }
    }

    font->needsUpdate = true;
}

static void FontStashRenderDraw(
    void *user, const float *vertices, const float *uvs, const uint32_t *colors, int32_t vertexCount)
{
    (void)colors;

    Font *font = (Font *)user;

    int32_t width = font->width;
    int32_t height = font->height;

    sgp_set_image(0, font->image);
    sgp_set_sampler(0, font->sampler);

    sgp_set_blend_mode(SGP_BLENDMODE_BLEND);

    for (int32_t i = 0; i < vertexCount - 2; i += 3)
    {
        int32_t q = 0;
        float x1 = vertices[(q + i) * 2];
        float y1 = vertices[(q + i) * 2 + 1];
        float u1 = uvs[(q + i) * 2];
        float v1 = uvs[(q + i) * 2 + 1];
        q += 2;
        float x3 = vertices[(q + i) * 2];
        float y3 = vertices[(q + i) * 2 + 1];
        float u3 = uvs[(q + i) * 2];
        float v3 = uvs[(q + i) * 2 + 1];

        sgp_draw_textured_rect(0, (sgp_rect){x1, y1, (x3 - x1), (y3 - y1)},
            (sgp_rect){u1 * width, v1 * height, (u3 - u1) * width, (v3 - v1) * height});
    }

    sgp_reset_image(0);
}

Font *FontNew(const char *path, float size)
{
    FILE *file = fopen(path, "rb");
    if (!file)
    {
        printf("Expected \"%s\"\n", path);
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    uint8_t *data = malloc(fileSize);
    fread(data, fileSize, 1, file);
    fclose(file);

    Font *font = malloc(sizeof(Font));
    *font = (Font){
        .atlasDimensions = FONT_ATLAS_SIZE,
        .size = size,
        .data = data,
    };

    FONSparams params = {
        .flags = FONS_ZERO_TOPLEFT,
        .width = font->atlasDimensions,
        .height = font->atlasDimensions,
        .userPtr = font,
        .renderCreate = FontStashRenderCreate,
        .renderDelete = FontStashRenderDelete,
        .renderResize = FontStashRenderResize,
        .renderUpdate = FontStashRenderUpdate,
        .renderDraw = FontStashRenderDraw,
    };
    font->context = fonsCreateInternal(&params);
    font->id = fonsAddFontMem(font->context, path, data, fileSize, false);

    return font;
}

int32_t FontDelete(Font *font)
{
    if (!font)
    {
        return 0;
    }

    free(font->data);
    fonsDeleteInternal(font->context);
    free(font);

    return 0;
}

void FontUpdate(Font *font)
{
    if (!font->needsUpdate)
    {
        return;
    }

    sg_image_data imageData = {0};
    imageData.subimage[0][0].ptr = font->buffer, imageData.subimage[0][0].size = font->width * font->height * 4,
    sg_update_image(font->image, &imageData);

    font->needsUpdate = false;
}

int32_t FontDraw(const char *text, float x, float y, Font *font)
{
    if (!text)
    {
        return 0;
    }

    if (!font)
    {
        fprintf(stderr, "No font set.\n");
        return -1;
    }

    if (font->id == FONS_INVALID)
    {
        fprintf(stderr, "Invalid font.\n");
        return -2;
    }

    fonsClearState(font->context);
    fonsSetFont(font->context, font->id);
    fonsSetSize(font->context, font->size);

    float fontHeight = 0.0;
    fonsVertMetrics(font->context, NULL, NULL, &fontHeight);
    fonsSetBlur(font->context, 0);
    fonsDrawText(font->context, x, y + fontHeight, text, NULL);

    return 0;
}

int32_t FontGetTextSize(
    const char *text, int32_t *width, int32_t *height, int32_t *ascent, int32_t *descent, Font *font)
{
    if (font->id == FONS_INVALID)
    {
        return -1;
    }

    float fonsWidth;
    float fonsHeight;
    float fonsAscent;
    float fonsDescent;
    fonsClearState(font->context);
    fonsSetFont(font->context, font->id);
    fonsSetSize(font->context, font->size);
    fonsWidth = fonsTextBounds(font->context, 0, 0, text, text + strlen(text), NULL);
    fonsVertMetrics(font->context, &fonsAscent, &fonsDescent, &fonsHeight);

    if (width)
    {
        *width = (int32_t)fonsWidth;
    }

    if (height)
    {
        *height = (int32_t)fonsHeight;
    }

    if (ascent)
    {
        *ascent = (int32_t)fonsAscent;
    }

    if (descent)
    {
        *descent = (int32_t)fonsDescent;
    }

    return 0;
}