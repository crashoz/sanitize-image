#include <stdio.h>
#include <string.h>
#include <sanitize-image.h>
#include "quantizers.h"
#include "converters.h"

#define MAX_PATH_LEN 4096

int szim_sanitize(unsigned char *data, size_t size, szim_image_type input_type, const char *path, szim_options_t options, char *res_path, size_t res_path_len)
{
    int ret, errorcode;
    szim_image_t *im = NULL;

    // debug_options(options);

    // Guess image type with the magic header
    if (input_type == SZIM_TYPE_UNKNOWN)
    {
        if (is_png(data, size))
        {
            input_type = SZIM_TYPE_PNG;
        }
        else if (is_jpeg(data, size))
        {
            input_type = SZIM_TYPE_JPEG;
        }
        else
        {
            return SZIM_ERROR_UNKNOWN_IMAGE_TYPE;
        }
    }

    // Check if type is allowed
    int i = 0;
    for (; i < 8; i++)
    {
        if (input_type == options.input.allowed_types[i])
        {
            break;
        }
    }

    if (i >= 8)
    {
        return SZIM_ERROR_NOT_ALLOWED_TYPE;
    }

    // Set output type
    if (options.output.type == SZIM_TYPE_INPUT)
    {
        options.output.type = input_type;
    }

    // Decode to internal format
    switch (input_type)
    {
    case SZIM_TYPE_PNG:
        ret = png_decode(data, size, options.input.max_width, options.input.max_height, options.input.max_size, &im);
        break;

    case SZIM_TYPE_JPEG:
        ret = jpeg_decode(data, size, options.input.max_width, options.input.max_height, options.input.max_size, &im);
        break;

    default:
        return SZIM_ERROR_UNKNOWN_IMAGE_TYPE;
    }

    if (ret != 0)
    {
        if (im != NULL)
        {
            destroy_image(im);
        }
        return ret;
    }

    // debug_image(im);

    // Randomize color values
    ret = 0;
    switch (options.randomizer.type)
    {
    case SZIM_RANDOMIZER_NONE:
        break;
    case SZIM_RANDOMIZER_AUTO:
        if (im->color == SZIM_COLOR_PALETTE)
        {
            ret = randomize_palette(im);
        }
        else
        {
            if (im->trns_len > 0)
            {
                ret = randomize_channels_keep_trns(im);
            }
            else
            {
                ret = randomize_channels(im);
            }
        }
        break;
    default:
        break;
    }

    if (ret != 0)
    {
        destroy_image(im);
        return ret;
    }

    // Resize image
    szim_image_t *resized_im;
    resize(im, &resized_im, options.resizer.width, options.resizer.height, options.resizer.type);
    if (im != resized_im)
    {
        destroy_image(im);
        im = resized_im;
    }

    // Convert
    szim_color_type output_color;
    switch (options.output.type)
    {
    case SZIM_TYPE_PNG:
        if (options.output.png.color_type == SZIM_TYPE_INPUT)
        {
            output_color = im->color;
        }
        else
        {
            output_color = options.output.png.color_type;
        }
        break;

    case SZIM_TYPE_JPEG:
        output_color = SZIM_COLOR_RGB;
        break;
    default:
        return SZIM_ERROR_NOT_SUPPORTED;
    }

    if (im->color != output_color)
    {
        // printf("convert %s -> %s\n", color_type_to_str(im->color), color_type_to_str(output_color));
        szim_image_t *converted_im;
        ret = convert_map[im->color][output_color](im, &converted_im);
        if (ret != 0)
        {
            return ret;
        }

        destroy_image(im);
        im = converted_im;
    }

    // im->trns_len = 3;
    // im->trns = malloc(3 * sizeof(uint16_t));
    // *((uint16_t *)(im->trns) + 0) = 205;
    // *((uint16_t *)(im->trns) + 1) = 95;
    // *((uint16_t *)(im->trns) + 2) = 96;

    // debug_image(im);

    // Encode to output file
    char full_path[MAX_PATH_LEN];
    strncpy(full_path, path, MAX_PATH_LEN - 8);

    char ext[8];
    type_to_ext(options.output.type, ext, 8);
    strncat(full_path, ext, 8);

    switch (options.output.type)
    {
    case SZIM_TYPE_PNG:
        png_encode(full_path, im, options.output.png);
        break;
    case SZIM_TYPE_JPEG:
        jpeg_encode(full_path, im, options.output.jpeg);
        break;
    default:
        break;
    }

    strncpy(res_path, full_path, res_path_len);

    destroy_image(im);
}