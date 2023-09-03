#include <stdio.h>
#include <sanitize-image.h>

options_t default_options()
{
    options_t options = {
        {{TYPE_PNG, TYPE_JPEG}, 1024, 1024, 1024 * 1024 * 3},
        {RANDOMIZER_DEFAULT},
        {RESIZER_NONE, 512, 512},
        {TYPE_JPEG, {}, {90}}};
    return options;
}

int sanitize(unsigned char *data, size_t size, image_type input_type, const char *path, options_t options)
{
    int ret, errorcode;
    image_t *im = NULL;

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
            return UNKNOWN_IMAGE_TYPE;
        }
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
        return UNKNOWN_IMAGE_TYPE;
    }

    if (ret != 0)
    {
        if (im != NULL)
        {
            free(im->data);
            free(im);
        }
        return ret;
    }

    printf("code: %d\n", ret);
    printf("size: (%d, %d)\n", im->width, im->height);

    // Randomize color values
    switch (options.randomizer.type)
    {
    case RANDOMIZER_NONE:
        break;
    case RANDOMIZER_AUTO:
    case RANDOMIZER_DEFAULT:
        randomize_rgb(im);
        break;
    default:
        break;
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

    // Encode to output file
    if (options.output.type == TYPE_INPUT)
    {
        options.output.type = input_type;
    }

    switch (options.output.type)
    {
    case TYPE_PNG:
        png_encode(path, im, SPNG_COLOR_TYPE_TRUECOLOR, 8);
        break;
    case TYPE_JPEG:
        jpeg_encode(path, im, options.output.jpeg.quality);
        break;
    default:
        break;
    }

    free(im->data);
    free(im);
}