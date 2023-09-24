#include <sanitize-image.h>
#include <quantizers.h>

const uint32_t x = 313524;
const uint32_t y = 615514;
const uint32_t z = 119537;
const uint32_t k = 1048576;

int rgb_to_rgba(image_t *src, image_t **dst)
{
    image_t *im = malloc(sizeof(image_t));
    if (im == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

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
        im->data[i * 4 + 3] = 255;
    }

    *dst = im;

    return SUCCESS;
}

int rgb_to_gray(image_t *src, image_t **dst)
{
    image_t *im = malloc(sizeof(image_t));
    if (im == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    im->color = COLOR_GRAY;
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

    for (int i = 0; i < src->width * src->height; i++)
    {

        im->data[i] = (src->data[i * 3] * x + src->data[i * 3 + 1] * y + src->data[i * 3 + 2] * z) / k;
    }

    *dst = im;

    return SUCCESS;
}

int rgb_to_graya(image_t *src, image_t **dst)
{
    image_t *im = malloc(sizeof(image_t));
    if (im == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    im->color = COLOR_GRAYA;
    im->bit_depth = 8;
    im->channels = 2;
    im->width = src->width;
    im->height = src->height;
    im->data = NULL;
    im->palette_len = 0;
    im->palette = NULL;
    im->trns_len = 0;
    im->trns = NULL;

    im->data = malloc(src->width * src->height * 2);
    if (im->data == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    for (int i = 0; i < src->width * src->height; i++)
    {

        im->data[i * 2] = (src->data[i * 3] * x + src->data[i * 3 + 1] * y + src->data[i * 3 + 2] * z) / k;
        im->data[i * 2 + 1] = 128;
    }

    *dst = im;

    return SUCCESS;
}

int rgb_to_palette(image_t *src, image_t **dst)
{
    int ret;
    int n_colors = 255;
    octree_node_t *octree = octree_create_node();
    if (octree == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    for (uint32_t i = 0; i < src->width * src->height; i++)
    {
        ret = octree_insert_color(octree, src->data[i * 3], src->data[i * 3 + 1], src->data[i * 3 + 2]);
        if (ret != 0)
        {
            return ret;
        }
    }

    heap_t *heap = octree_to_heap(octree);
    if (heap == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    octree_reduce(heap, n_colors);

    unsigned char *indexed_data = malloc(src->width * src->height);
    if (indexed_data == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    unsigned char *palette = malloc(n_colors * 3);
    if (palette == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    int plte_len;
    octree_palette(src, octree, n_colors, indexed_data, palette, &plte_len);

    ret = dither(src, palette, plte_len, indexed_data);
    if (ret != 0)
    {
        return ret;
    }

    image_t *im = malloc(sizeof(image_t));
    if (im == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    im->color = COLOR_PALETTE;
    im->bit_depth = 8;
    im->channels = 1;
    im->width = src->width;
    im->height = src->height;
    im->data = indexed_data;
    im->palette_len = plte_len;
    im->palette = palette;
    im->trns_len = 0;
    im->trns = NULL;

    heap_destroy(heap);
    octree_destroy(octree);

    *dst = im;
    return SUCCESS;
}

int rgba_to_rgb(image_t *src, image_t **dst)
{
    image_t *im = malloc(sizeof(image_t));
    if (im == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    im->color = COLOR_RGB;
    im->bit_depth = 8;
    im->channels = 3;
    im->width = src->width;
    im->height = src->height;
    im->data = NULL;
    im->palette_len = 0;
    im->palette = NULL;
    im->trns_len = 0;
    im->trns = NULL;

    im->data = malloc(src->width * src->height * 3);
    if (im->data == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    const uint8_t bg = 255;

    for (int i = 0; i < src->width * src->height; i++)
    {
        im->data[i * 3] = (src->data[i * 4 + 3] * src->data[i * 4] + (255 - src->data[i * 4 + 3]) * bg) / 256;
        im->data[i * 3 + 1] = (src->data[i * 4 + 3] * src->data[i * 4 + 1] + (255 - src->data[i * 4 + 3]) * bg) / 256;
        im->data[i * 3 + 2] = (src->data[i * 4 + 3] * src->data[i * 4 + 2] + (255 - src->data[i * 4 + 3]) * bg) / 256;
    }

    *dst = im;

    return SUCCESS;
}

int gray_to_rgb(image_t *src, image_t **dst)
{
    image_t *im = malloc(sizeof(image_t));
    if (im == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    im->color = COLOR_RGB;
    im->bit_depth = 8;
    im->channels = 3;
    im->width = src->width;
    im->height = src->height;
    im->data = NULL;
    im->palette_len = 0;
    im->palette = NULL;
    im->trns_len = 0;
    im->trns = NULL;

    im->data = malloc(src->width * src->height * 3);
    if (im->data == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    for (int i = 0; i < src->width * src->height; i++)
    {

        im->data[i * 3] = src->data[i];
        im->data[i * 3 + 1] = src->data[i];
        im->data[i * 3 + 2] = src->data[i];
    }

    *dst = im;

    return SUCCESS;
}

int graya_to_rgb(image_t *src, image_t **dst)
{
    image_t *im = malloc(sizeof(image_t));
    if (im == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    im->color = COLOR_RGB;
    im->bit_depth = 8;
    im->channels = 3;
    im->width = src->width;
    im->height = src->height;
    im->data = NULL;
    im->palette_len = 0;
    im->palette = NULL;
    im->trns_len = 0;
    im->trns = NULL;

    im->data = malloc(src->width * src->height * 3);
    if (im->data == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    const uint8_t bg = 255;

    for (int i = 0; i < src->width * src->height; i++)
    {
        uint8_t color = (src->data[i * 2 + 1] * src->data[i * 2] + (255 - src->data[i * 2 + 1]) * bg) / 256;
        im->data[i * 3] = color;
        im->data[i * 3 + 1] = color;
        im->data[i * 3 + 2] = color;
    }

    *dst = im;

    return SUCCESS;
}

int palette_to_rgb(image_t *src, image_t **dst)
{
    image_t *im = malloc(sizeof(image_t));
    if (im == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    im->color = COLOR_RGB;
    im->bit_depth = 8;
    im->channels = 3;
    im->width = src->width;
    im->height = src->height;
    im->data = NULL;
    im->palette_len = 0;
    im->palette = NULL;
    im->trns_len = 0;
    im->trns = NULL;

    im->data = malloc(src->width * src->height * 3);
    if (im->data == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    for (int i = 0; i < src->width * src->height; i++)
    {

        im->data[i * 3] = src->palette[src->data[i] * 3];
        im->data[i * 3 + 1] = src->palette[src->data[i] * 3 + 1];
        im->data[i * 3 + 2] = src->palette[src->data[i] * 3 + 2];
    }

    *dst = im;

    return SUCCESS;
}

// TODO all convertions!

int gray_to_graya(image_t *src, image_t **dst)
{
    // TODO test trns
    image_t *im = malloc(sizeof(image_t));
    if (im == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    im->color = COLOR_GRAYA;
    im->bit_depth = 8;
    im->channels = 2;
    im->width = src->width;
    im->height = src->height;
    im->data = NULL;
    im->palette_len = 0;
    im->palette = NULL;
    im->trns_len = 0;
    im->trns = NULL;

    im->data = malloc(src->width * src->height * im->channels);
    if (im->data == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    if (src->trns_len == 0)
    {
        for (int i = 0; i < src->width * src->height; i++)
        {
            im->data[i * 2] = src->data[i];
            im->data[i * 2 + 1] = 255;
        }
    }
    else
    {
        unsigned char transparent = *((uint16_t *)(im->trns));
        for (int i = 0; i < src->width * src->height; i++)
        {
            im->data[i * 2] = src->data[i];

            if (src->data[i] == transparent)
            {
                im->data[i * 2 + 1] = 0;
            }
            else
            {
                im->data[i * 2 + 1] = 255;
            }
        }
    }

    *dst = im;

    return SUCCESS;
}