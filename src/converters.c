#include <sanitize-image.h>

int rgb_to_rgba(image_t *src, image_t **dst)
{
    image_t *im = malloc(sizeof(image_t));

    im->color = COLOR_RGBA;
    im->bit_depth = 8;
    im->channels = 4;
    im->width = src->width;
    im->height = src->height;
    im->data = NULL;
    im->palette_len = 0;
    im->palette = NULL;
    im->trns_len = 0;
    im->trns = NULL;

    im->data = malloc(src->width * src->height * 4);
    if (im->data == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    for (int i = 0; i < src->width * src->height; i++)
    {
        im->data[i * 4] = src->data[i * 3];
        im->data[i * 4 + 1] = src->data[i * 3 + 1];
        im->data[i * 4 + 2] = src->data[i * 3 + 2];
        im->data[i * 4 + 3] = 128;
    }

    *dst = im;

    return SUCCESS;
}

int rgb_to_grayscale(image_t *src, image_t **dst)
{
    image_t *im = malloc(sizeof(image_t));

    im->color = COLOR_GRAYSCALE;
    im->bit_depth = 8;
    im->channels = 1;
    im->width = src->width;
    im->height = src->height;
    im->data = NULL;
    im->palette_len = 0;
    im->palette = NULL;
    im->trns_len = 0;
    im->trns = NULL;

    im->data = malloc(src->width * src->height * 1);
    if (im->data == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    const uint32_t x = 313524;
    const uint32_t y = 615514;
    const uint32_t z = 119537;
    const uint32_t k = 1048576;

    for (int i = 0; i < src->width * src->height; i++)
    {

        im->data[i] = (src->data[i * 3] * x + src->data[i * 3 + 1] * y + src->data[i * 3 + 2] * z) / k;
    }

    *dst = im;

    return SUCCESS;
}