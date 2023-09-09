#include <stdio.h>
#include <string.h>
#include <sanitize-image.h>

image_type str_to_type(const char *str)
{
    if (strcmp(str, "png") == 0)
    {
        return TYPE_PNG;
    }
    if (strcmp(str, "jpeg") == 0 || strcmp(str, "jpg") == 0)
    {
        return TYPE_JPEG;
    }
    if (strcmp(str, "input") == 0)
    {
        return TYPE_INPUT;
    }
    return TYPE_UNKNOWN;
}

int type_to_str(image_type type, char *str, size_t len)
{
    if (len < 8)
    {
        return 1;
    }

    switch (type)
    {
    case TYPE_UNKNOWN:
        strncpy(str, "unknown", 8);
        break;
    case TYPE_INPUT:
        strncpy(str, "input", 6);
        break;
    case TYPE_PNG:
        strncpy(str, "png", 4);
        break;
    case TYPE_JPEG:
        strncpy(str, "jpeg", 5);
    default:
        return 1;
    }

    return 0;
}

int type_to_ext(image_type type, char *str, size_t len)
{
    if (len < 5)
    {
        return 1;
    }

    switch (type)
    {
    case TYPE_PNG:
        strncpy(str, ".png", 5);
        break;
    case TYPE_JPEG:
        strncpy(str, ".jpg", 5);
    default:
        return 1;
    }

    return 0;
}

color_type png_to_color_type(enum spng_color_type color)
{
    switch (color)
    {
    case SPNG_COLOR_TYPE_GRAYSCALE:
        return COLOR_GRAYSCALE;
    case SPNG_COLOR_TYPE_GRAYSCALE_ALPHA:
        return COLOR_GRAYSCALE_ALPHA;
    case SPNG_COLOR_TYPE_TRUECOLOR:
        return COLOR_RGB;
    case SPNG_COLOR_TYPE_TRUECOLOR_ALPHA:
        return COLOR_RGBA;
    case SPNG_COLOR_TYPE_INDEXED:
        return COLOR_PALETTE;
    default:
        return COLOR_UNKNOWN;
    }
}

int color_type_to_channels(color_type color)
{
    switch (color)
    {
    case COLOR_GRAYSCALE:
        return 1;
    case COLOR_GRAYSCALE_ALPHA:
        return 2;
    case COLOR_RGB:
        return 3;
    case COLOR_RGBA:
        return 4;
    }
}

options_t default_options()
{
    options_t options = {
        {{TYPE_PNG, TYPE_JPEG}, 1024, 1024, 1024 * 1024 * 3},
        {RANDOMIZER_AUTO},
        {RESIZER_NONE, 512, 512},
        {TYPE_JPEG, {}, {90}}};
    return options;
}

void debug_options(options_t options)
{
    printf("input:\n");
    printf("\tallowed_inputs: [");
    for (int i = 0; i < 8; i++)
    {
        if (options.input.allowed_types[i] == TYPE_UNKNOWN)
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
    printf("\tjpeg:\n");
    printf("\t\tquality: %d\n", options.output.jpeg.quality);
    printf("\n");
}
