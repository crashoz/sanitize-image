#include <sanitize-image.h>
#include <quantizers.h>
#include <converters.h>

const uint32_t x = 313524;
const uint32_t y = 615514;
const uint32_t z = 119537;
const uint32_t k = 1048576;

const uint8_t bg = 0;

uint8_t apply_alpha(uint8_t c, uint8_t alpha)
{
    return (alpha * c + (255 - alpha) * bg) / 256;
}

uint8_t grayscalize(uint8_t r, uint8_t g, uint8_t b)
{
    return (r * x + g * y + b * z) / k;
}

convert_fn convert_map[5][5];

void init_convert_map()
{
    convert_map[COLOR_GRAY][COLOR_GRAY] = &identity;
    convert_map[COLOR_GRAY][COLOR_GRAYA] = &gray_to_graya;
    convert_map[COLOR_GRAY][COLOR_RGB] = &gray_to_rgb;
    convert_map[COLOR_GRAY][COLOR_RGBA] = &gray_to_rgba;
    convert_map[COLOR_GRAY][COLOR_PALETTE] = &gray_to_palette;

    convert_map[COLOR_GRAYA][COLOR_GRAY] = &graya_to_gray;
    convert_map[COLOR_GRAYA][COLOR_GRAYA] = &identity;
    convert_map[COLOR_GRAYA][COLOR_RGB] = &graya_to_rgb;
    convert_map[COLOR_GRAYA][COLOR_RGBA] = &graya_to_rgba;
    convert_map[COLOR_GRAYA][COLOR_PALETTE] = &graya_to_palette;

    convert_map[COLOR_RGB][COLOR_GRAY] = &rgb_to_gray;
    convert_map[COLOR_RGB][COLOR_GRAYA] = &rgb_to_graya;
    convert_map[COLOR_RGB][COLOR_RGB] = &identity;
    convert_map[COLOR_RGB][COLOR_RGBA] = &rgb_to_rgba;
    convert_map[COLOR_RGB][COLOR_PALETTE] = &rgb_to_palette;

    convert_map[COLOR_RGBA][COLOR_GRAY] = &rgba_to_gray;
    convert_map[COLOR_RGBA][COLOR_GRAYA] = &rgba_to_graya;
    convert_map[COLOR_RGBA][COLOR_RGB] = &rgba_to_rgb;
    convert_map[COLOR_RGBA][COLOR_RGBA] = &identity;
    convert_map[COLOR_RGBA][COLOR_PALETTE] = &rgba_to_palette;

    convert_map[COLOR_PALETTE][COLOR_GRAY] = &palette_to_gray;
    convert_map[COLOR_PALETTE][COLOR_GRAYA] = &palette_to_graya;
    convert_map[COLOR_PALETTE][COLOR_RGB] = &palette_to_rgb;
    convert_map[COLOR_PALETTE][COLOR_RGBA] = &palette_to_rgba;
    convert_map[COLOR_PALETTE][COLOR_PALETTE] = &identity;
}

// TODO test all

int identity(image_t *src, image_t **dst)
{
    image_t *im = malloc(sizeof(image_t));
    if (im == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    im_deep_copy(src, im);
    *dst = im;
}

/* GRAY */

int gray_to_graya(image_t *src, image_t **dst)
{
    printf("is here!\n");
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

    im->data = malloc(src->width * src->height * im->channels);
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

int gray_to_rgba(image_t *src, image_t **dst)
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

    im->data = malloc(src->width * src->height * im->channels);
    if (im->data == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    if (src->trns_len == 0)
    {
        for (int i = 0; i < src->width * src->height; i++)
        {
            im->data[i * 4] = src->data[i];
            im->data[i * 4 + 1] = src->data[i];
            im->data[i * 4 + 2] = src->data[i];
            im->data[i * 4 + 3] = 255;
        }
    }
    else
    {
        unsigned char transparent = *((uint16_t *)(im->trns));
        for (int i = 0; i < src->width * src->height; i++)
        {
            im->data[i * 4] = src->data[i];
            im->data[i * 4 + 1] = src->data[i];
            im->data[i * 4 + 2] = src->data[i];

            if (src->data[i] == transparent)
            {
                im->data[i * 4 + 3] = 0;
            }
            else
            {
                im->data[i * 4 + 3] = 255;
            }
        }
    }

    *dst = im;

    return SUCCESS;
}

int gray_to_palette(image_t *src, image_t **dst)
{
    int ret;
    image_t *im_rgb;

    ret = gray_to_rgb(src, &im_rgb);
    if (ret != 0)
    {
        return ret;
    }

    ret = rgb_to_palette(im_rgb, dst);
    destroy_image(im_rgb);
    return ret;
}

/* GRAYA */

int graya_to_gray(image_t *src, image_t **dst)
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

    im->data = malloc(src->width * src->height * im->channels);
    if (im->data == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    for (int i = 0; i < src->width * src->height; i++)
    {
        uint8_t color = apply_alpha(src->data[i * 2], src->data[i * 2 + 1]);
        im->data[i] = color;
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

    im->data = malloc(src->width * src->height * im->channels);
    if (im->data == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    for (int i = 0; i < src->width * src->height; i++)
    {
        uint8_t color = apply_alpha(src->data[i * 2], src->data[i * 2 + 1]);
        im->data[i * 3] = color;
        im->data[i * 3 + 1] = color;
        im->data[i * 3 + 2] = color;
    }

    *dst = im;

    return SUCCESS;
}

int graya_to_rgba(image_t *src, image_t **dst)
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

    im->data = malloc(src->width * src->height * im->channels);
    if (im->data == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    for (int i = 0; i < src->width * src->height; i++)
    {
        im->data[i * 4] = src->data[i * 2];
        im->data[i * 4 + 1] = src->data[i * 2];
        im->data[i * 4 + 2] = src->data[i * 2];
        im->data[i * 4 + 3] = src->data[i * 2 + 1];
    }

    *dst = im;

    return SUCCESS;
}

int graya_to_palette(image_t *src, image_t **dst)
{
    int ret;
    image_t *im_rgb;

    ret = graya_to_rgb(src, &im_rgb);
    if (ret != 0)
    {
        return ret;
    }

    ret = rgb_to_palette(im_rgb, dst);
    destroy_image(im_rgb);
    return ret;
}

/* RGB */

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

    im->data = malloc(src->width * src->height * im->channels);
    if (im->data == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    for (int i = 0; i < src->width * src->height; i++)
    {

        im->data[i] = grayscalize(src->data[i * 3], src->data[i * 3 + 1], src->data[i * 3 + 2]);
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

    im->data = malloc(src->width * src->height * im->channels);
    if (im->data == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    for (int i = 0; i < src->width * src->height; i++)
    {

        im->data[i * 2] = grayscalize(src->data[i * 3], src->data[i * 3 + 1], src->data[i * 3 + 2]);
        im->data[i * 2 + 1] = 255;
    }

    *dst = im;

    return SUCCESS;
}

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

    im->data = malloc(src->width * src->height * im->channels);
    if (im->data == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    if (src->trns_len == 0)
    {
        for (int i = 0; i < src->width * src->height; i++)
        {
            im->data[i * 4] = src->data[i * 3];
            im->data[i * 4 + 1] = src->data[i * 3 + 1];
            im->data[i * 4 + 2] = src->data[i * 3 + 2];
            im->data[i * 4 + 3] = 255;
        }
    }
    else
    {
        uint8_t transparent_r = *((uint16_t *)(src->trns));
        uint8_t transparent_g = *((uint16_t *)(src->trns) + 1);
        uint8_t transparent_b = *((uint16_t *)(src->trns) + 2);
        for (int i = 0; i < src->width * src->height; i++)
        {
            im->data[i * 4] = src->data[i * 3];
            im->data[i * 4 + 1] = src->data[i * 3 + 1];
            im->data[i * 4 + 2] = src->data[i * 3 + 2];

            if (src->data[i * 3] == transparent_r && src->data[i * 3 + 1] == transparent_g && src->data[i * 3 + 2] == transparent_b)
            {
                im->data[i * 4 + 3] = 0;
            }
            else
            {
                im->data[i * 4 + 3] = 255;
            }
        }
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

/* RGBA */

int rgba_to_gray(image_t *src, image_t **dst)
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

    im->data = malloc(src->width * src->height * im->channels);
    if (im->data == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    for (int i = 0; i < src->width * src->height; i++)
    {
        uint8_t r = apply_alpha(src->data[i * 4], src->data[i * 4 + 3]);
        uint8_t g = apply_alpha(src->data[i * 4 + 1], src->data[i * 4 + 3]);
        uint8_t b = apply_alpha(src->data[i * 4 + 2], src->data[i * 4 + 3]);
        im->data[i] = grayscalize(r, g, b);
    }

    *dst = im;

    return SUCCESS;
}

int rgba_to_graya(image_t *src, image_t **dst)
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

    im->data = malloc(src->width * src->height * im->channels);
    if (im->data == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    for (int i = 0; i < src->width * src->height; i++)
    {
        im->data[i * 2] = grayscalize(src->data[i * 4], src->data[i * 4 + 1], src->data[i * 4 + 2]);
        im->data[i * 2 + 1] = src->data[i * 4 + 3];
    }

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

    im->data = malloc(src->width * src->height * im->channels);
    if (im->data == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    for (int i = 0; i < src->width * src->height; i++)
    {
        im->data[i * 3] = apply_alpha(src->data[i * 4], src->data[i * 4 + 3]);
        im->data[i * 3 + 1] = apply_alpha(src->data[i * 4 + 1], src->data[i * 4 + 3]);
        im->data[i * 3 + 2] = apply_alpha(src->data[i * 4 + 2], src->data[i * 4 + 3]);
    }

    *dst = im;

    return SUCCESS;
}

int rgba_to_palette(image_t *src, image_t **dst)
{
    int ret;
    image_t *im_rgb;

    ret = rgba_to_rgb(src, &im_rgb);
    if (ret != 0)
    {
        return ret;
    }

    ret = rgb_to_palette(im_rgb, dst);
    destroy_image(im_rgb);
    return ret;
}

/* PALETTE */

int palette_to_gray(image_t *src, image_t **dst)
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

    im->data = malloc(src->width * src->height * im->channels);
    if (im->data == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    if (src->trns == 0)
    {
        for (int i = 0; i < src->width * src->height; i++)
        {
            im->data[i] = grayscalize(
                src->palette[src->data[i] * 3],
                src->palette[src->data[i] * 3 + 1],
                src->palette[src->data[i] * 3 + 2]);
        }
    }
    else
    {
        for (int i = 0; i < src->width * src->height; i++)
        {
            uint8_t r = apply_alpha(src->palette[src->data[i] * 3], src->trns[src->data[i]]);
            uint8_t g = apply_alpha(src->palette[src->data[i] * 3 + 1], src->trns[src->data[i]]);
            uint8_t b = apply_alpha(src->palette[src->data[i] * 3 + 2], src->trns[src->data[i]]);
            im->data[i] = grayscalize(r, g, b);
        }
    }

    *dst = im;

    return SUCCESS;
}

int palette_to_graya(image_t *src, image_t **dst)
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

    im->data = malloc(src->width * src->height * im->channels);
    if (im->data == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    if (src->trns_len == 0)
    {
        for (int i = 0; i < src->width * src->height; i++)
        {
            im->data[i * 2] = grayscalize(
                src->palette[src->data[i] * 3],
                src->palette[src->data[i] * 3 + 1],
                src->palette[src->data[i] * 3 + 2]);
            im->data[i * 2 + 1] = 255;
        }
    }
    else
    {
        for (int i = 0; i < src->width * src->height; i++)
        {
            im->data[i * 2] = grayscalize(
                src->palette[src->data[i] * 3],
                src->palette[src->data[i] * 3 + 1],
                src->palette[src->data[i] * 3 + 2]);
            im->data[i * 2 + 1] = src->trns[src->data[i]];
        }
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

    im->data = malloc(src->width * src->height * im->channels);
    if (im->data == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    if (src->trns == 0)
    {
        for (int i = 0; i < src->width * src->height; i++)
        {
            im->data[i * 3] = src->palette[src->data[i] * 3];
            im->data[i * 3 + 1] = src->palette[src->data[i] * 3 + 1];
            im->data[i * 3 + 2] = src->palette[src->data[i] * 3 + 2];
        }
    }
    else
    {
        for (int i = 0; i < src->width * src->height; i++)
        {
            im->data[i * 3] = apply_alpha(src->palette[src->data[i] * 3], src->trns[src->data[i]]);
            im->data[i * 3 + 1] = apply_alpha(src->palette[src->data[i] * 3 + 1], src->trns[src->data[i]]);
            im->data[i * 3 + 2] = apply_alpha(src->palette[src->data[i] * 3 + 2], src->trns[src->data[i]]);
        }
    }

    *dst = im;

    return SUCCESS;
}

int palette_to_rgba(image_t *src, image_t **dst)
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

    im->data = malloc(src->width * src->height * im->channels);
    if (im->data == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    if (src->trns == 0)
    {
        for (int i = 0; i < src->width * src->height; i++)
        {
            im->data[i * 4] = src->palette[src->data[i] * 3];
            im->data[i * 4 + 1] = src->palette[src->data[i] * 3 + 1];
            im->data[i * 4 + 2] = src->palette[src->data[i] * 3 + 2];
            im->data[i * 4 + 3] = 255;
        }
    }
    else
    {
        for (int i = 0; i < src->width * src->height; i++)
        {
            im->data[i * 4] = src->palette[src->data[i] * 3];
            im->data[i * 4 + 1] = src->palette[src->data[i] * 3 + 1];
            im->data[i * 4 + 2] = src->palette[src->data[i] * 3 + 2];
            im->data[i * 4 + 3] = src->trns[src->data[i]];
        }
    }

    *dst = im;

    return SUCCESS;
}