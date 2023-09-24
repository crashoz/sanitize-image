#include <stdio.h>
#include <string.h>
#include <sanitize-image.h>
#include <quantizers.h>

#define MAX_PATH_LEN 4096

int sanitize(unsigned char *data, size_t size, image_type input_type, const char *path, options_t options, char *res_path, size_t res_path_len)
{
    int ret, errorcode;
    image_t *im = NULL;

    debug_options(options);

    // Guess image type with the magic header
    if (input_type == TYPE_UNKNOWN)
    {
        if (is_png(data, size))
        {
            input_type = TYPE_PNG;
        }
        else if (is_jpeg(data, size))
        {
            input_type = TYPE_JPEG;
        }
        else
        {
            return ERROR_UNKNOWN_IMAGE_TYPE;
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
        return ERROR_NOT_ALLOWED_TYPE;
    }

    // Decode to internal format
    switch (input_type)
    {
    case TYPE_PNG:
        ret = png_decode(data, size, options.input.max_width, options.input.max_height, options.input.max_size, &im);
        break;

    case TYPE_JPEG:
        ret = jpeg_decode(data, size, options.input.max_width, options.input.max_height, options.input.max_size, &im);
        break;

    default:
        return ERROR_UNKNOWN_IMAGE_TYPE;
    }

    printf("decode ret: %d\n", ret);

    if (ret != 0)
    {
        if (im != NULL)
        {
            free(im->data);
            free(im);
        }
        return ret;
    }

    debug_image(im);

    // Randomize color values
    ret = 0;
    switch (options.randomizer.type)
    {
    case RANDOMIZER_NONE:
        break;
    case RANDOMIZER_AUTO:
        if (im->color == COLOR_PALETTE)
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
        free(im->data);
        free(im);
        return ret;
    }

    // Resize image
    image_t *resized_im;
    resize(im, &resized_im, options.resizer.width, options.resizer.height, options.resizer.type);
    if (im != resized_im)
    {
        free(im->data);
        free(im);
        im = resized_im;
    }

    // Convert
    image_t *converted_im;
    gray_to_graya(im, &converted_im);
    free(im->data);
    free(im);
    im = converted_im;

    debug_image(im);

    // Encode to output file
    if (options.output.type == TYPE_INPUT)
    {
        options.output.type = input_type;
    }

    char full_path[MAX_PATH_LEN];
    strncpy(full_path, path, MAX_PATH_LEN - 8);

    char ext[8];
    type_to_ext(options.output.type, ext, 8);
    strncat(full_path, ext, 8);

    switch (options.output.type)
    {
    case TYPE_PNG:
        png_encode(full_path, im, options.output.png);
        break;
    case TYPE_JPEG:
        jpeg_encode(full_path, im, options.output.jpeg);
        break;
    default:
        break;
    }

    strncpy(res_path, full_path, res_path_len);

    free(im->data);
    free(im);
}