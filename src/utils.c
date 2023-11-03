#include <stdio.h>
#include <string.h>
#include <sanitize-image.h>

void destroy_image(szim_image_t *im)
{
    free(im->data);
    free(im->palette);
    free(im->trns);
    free(im);
}

szim_image_type str_to_type(const char *str)
{
    if (strcmp(str, "png") == 0)
    {
        return SZIM_TYPE_PNG;
    }
    if (strcmp(str, "jpeg") == 0 || strcmp(str, "jpg") == 0)
    {
        return SZIM_TYPE_JPEG;
    }
    if (strcmp(str, "input") == 0)
    {
        return SZIM_TYPE_INPUT;
    }
    return SZIM_TYPE_UNKNOWN;
}

int type_to_str(szim_image_type type, char *str, size_t len)
{
    if (len < 8)
    {
        return 1;
    }

    switch (type)
    {
    case SZIM_TYPE_UNKNOWN:
        strncpy(str, "unknown", 8);
        break;
    case SZIM_TYPE_INPUT:
        strncpy(str, "input", 6);
        break;
    case SZIM_TYPE_PNG:
        strncpy(str, "png", 4);
        break;
    case SZIM_TYPE_JPEG:
        strncpy(str, "jpeg", 5);
    default:
        return 1;
    }

    return 0;
}

int type_to_ext(szim_image_type type, char *str, size_t len)
{
    if (len < 5)
    {
        return 1;
    }

    switch (type)
    {
    case SZIM_TYPE_PNG:
        strncpy(str, ".png", 5);
        break;
    case SZIM_TYPE_JPEG:
        strncpy(str, ".jpg", 5);
    default:
        return 1;
    }

    return 0;
}

szim_color_type png_to_color_type(enum spng_color_type color)
{
    switch (color)
    {
    case SPNG_COLOR_TYPE_GRAYSCALE:
        return SZIM_COLOR_GRAY;
    case SPNG_COLOR_TYPE_GRAYSCALE_ALPHA:
        return SZIM_COLOR_GRAYA;
    case SPNG_COLOR_TYPE_TRUECOLOR:
        return SZIM_COLOR_RGB;
    case SPNG_COLOR_TYPE_TRUECOLOR_ALPHA:
        return SZIM_COLOR_RGBA;
    case SPNG_COLOR_TYPE_INDEXED:
        return SZIM_COLOR_PALETTE;
    default:
        return SZIM_COLOR_UNKNOWN;
    }
}

enum spng_color_type color_type_to_png(szim_color_type color)
{
    switch (color)
    {
    case SZIM_COLOR_GRAY:
        return SPNG_COLOR_TYPE_GRAYSCALE;
    case SZIM_COLOR_GRAYA:
        return SPNG_COLOR_TYPE_GRAYSCALE_ALPHA;
    case SZIM_COLOR_RGB:
        return SPNG_COLOR_TYPE_TRUECOLOR;
    case SZIM_COLOR_RGBA:
        return SPNG_COLOR_TYPE_TRUECOLOR_ALPHA;
    case SZIM_COLOR_PALETTE:
        return SPNG_COLOR_TYPE_INDEXED;
    default:
        return SPNG_COLOR_TYPE_TRUECOLOR;
    }
}

const char *color_type_to_str(szim_color_type color)
{
    switch (color)
    {
    case SZIM_COLOR_GRAY:
        return "gray";
    case SZIM_COLOR_GRAYA:
        return "graya";
    case SZIM_COLOR_RGB:
        return "rgb";
    case SZIM_COLOR_RGBA:
        return "rgba";
    case SZIM_COLOR_PALETTE:
        return "palette";
    default:
        return "unknown";
    }
}

szim_color_type str_to_color_type(const char *str)
{
    if (strcmp(str, "gray") == 0)
    {
        return SZIM_COLOR_GRAY;
    }
    else if (strcmp(str, "graya") == 0)
    {
        return SZIM_COLOR_GRAYA;
    }
    else if (strcmp(str, "rgb") == 0)
    {
        return SZIM_COLOR_RGB;
    }
    else if (strcmp(str, "rgba") == 0)
    {
        return SZIM_COLOR_RGBA;
    }
    else if (strcmp(str, "palette") == 0)
    {
        return SZIM_COLOR_PALETTE;
    }
    return SZIM_COLOR_UNKNOWN;
}

int color_type_to_channels(szim_color_type color)
{
    switch (color)
    {
    case SZIM_COLOR_GRAY:
        return 1;
    case SZIM_COLOR_GRAYA:
        return 2;
    case SZIM_COLOR_RGB:
        return 3;
    case SZIM_COLOR_RGBA:
        return 4;
    case SZIM_COLOR_PALETTE:
        return 1;
    }
}

szim_options_t szim_default_options()
{
    szim_options_t options = {
        {{SZIM_TYPE_PNG, SZIM_TYPE_JPEG}, 1024, 1024, 1024 * 1024 * 3},
        {SZIM_RANDOMIZER_AUTO},
        {SZIM_RESIZER_NONE, 512, 512},
        {SZIM_TYPE_INPUT,
         {SZIM_COLOR_INPUT, 9, SPNG_FILTER_CHOICE_ALL, false},
         {90, false, JDCT_DEFAULT, true, 0, false}}};
    return options;
}

void im_shallow_copy(szim_image_t *src, szim_image_t *dst)
{
    memcpy(dst, src, sizeof(szim_image_t));
    dst->data = NULL;
    dst->palette = NULL;
    dst->trns = NULL;
}

void im_deep_copy(szim_image_t *src, szim_image_t *dst)
{
    memcpy(dst, src, sizeof(szim_image_t));
    dst->data = malloc(src->width * src->height * src->channels);
    memcpy(dst->data, src->data, src->width * src->height * src->channels);

    if (src->palette_len > 0)
    {
        dst->palette = malloc(src->palette_len * 3);
        memcpy(dst->palette, src->palette, src->palette_len * 3);
    }

    if (src->trns_len > 0)
    {
        if (src->trns_len <= 3)
        {
            dst->trns = malloc(src->trns_len * 2);
            memcpy(dst->trns, src->trns, src->trns_len * 2);
        }
        else
        {
            dst->trns = malloc(src->trns_len);
            memcpy(dst->trns, src->trns, src->trns_len);
        }
    }
}

void debug_options(szim_options_t options)
{
    printf("input:\n");
    printf("\tallowed_inputs: [");
    for (int i = 0; i < 8; i++)
    {
        if (options.input.allowed_types[i] == SZIM_TYPE_UNKNOWN)
        {
            printf("]\n");
            break;
        }
        if (i > 0)
        {
            printf(", ");
        }
        printf("%d", options.input.allowed_types[i]);
    }
    printf("\tmax_width: %d\n", options.input.max_width);
    printf("\tmax_height: %d\n", options.input.max_height);
    printf("\tmax_size: %d\n", options.input.max_size);

    printf("randomizer:\n");
    printf("\ttype: %d\n", options.randomizer.type);

    printf("resizer:\n");
    printf("\ttype: %d\n", options.resizer.type);
    printf("\twidth: %d\n", options.resizer.width);
    printf("\theight: %d\n", options.resizer.height);

    printf("output:\n");
    printf("\ttype: %d\n", options.output.type);
    printf("\tpng:\n");
    printf("\t\tcolor: %s\n", color_type_to_str(options.output.png.color_type));
    printf("\t\tcompression: %d\n", options.output.png.compression_level);
    printf("\t\tfilter: %d\n", options.output.png.filter);
    printf("\t\tinterlace: %d\n", options.output.png.interlace);
    printf("\tjpeg:\n");
    printf("\t\tquality: %d\n", options.output.jpeg.quality);
    printf("\t\tarith_code: %d\n", options.output.jpeg.arith_code);
    printf("\t\tdct_method: %d\n", options.output.jpeg.dct_method);
    printf("\t\toptimize: %d\n", options.output.jpeg.optimize);
    printf("\t\tsmoothing: %d\n", options.output.jpeg.smoothing);
    printf("\n");
}

void debug_image(szim_image_t *im)
{
    printf("image:\n");

    printf("\tcolor: %s\n", color_type_to_str(im->color));
    printf("\tbit_depth: %d\n", im->bit_depth);
    printf("\tchannels: %d\n", im->channels);
    printf("\twidth: %d\n", im->width);
    printf("\theight: %d\n", im->height);
    printf("\tpalette_len: %d\n", im->palette_len);
    printf("\ttrns_len: %d\n", im->trns_len);
    printf("\n");
}