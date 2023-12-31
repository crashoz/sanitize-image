#include <string.h>
#include <sanitize-image.h>

#define FACTOR 2048
#define SHIFT 11

int resize(szim_image_t *src, szim_image_t **dst_ptr, uint32_t width, uint32_t height, szim_resizer_type type)
{
    if (type == SZIM_RESIZER_NONE)
    {
        *dst_ptr = src;
        return 0;
    }

    if (type == SZIM_RESIZER_AUTO)
    {
        if (src->palette != NULL)
        {
            type = SZIM_RESIZER_NN;
        }
        else
        {
            type = SZIM_RESIZER_BILINEAR;
        }
    }

    if (src->width == width && src->height == height)
    {
        *dst_ptr = src;
        return 0;
    }

    if (width == 0)
    {
        width = (uint32_t)((src->width * height) / src->height);
    }

    if (height == 0)
    {
        height = (uint32_t)((src->height * width) / src->width);
    }

    switch (type)
    {
    case SZIM_RESIZER_NN:
        nn_interp(src, dst_ptr, width, height);
        break;
    case SZIM_RESIZER_BILINEAR:
        bilinear_interp(src, dst_ptr, width, height);
        break;
    default:
        break;
    }

    return 0;
}

int nn_interp(szim_image_t *src, szim_image_t **dst_ptr, uint32_t width, uint32_t height)
{
    szim_image_t *dst = malloc(sizeof(szim_image_t));
    if (dst == NULL)
    {
        return SZIM_ERROR_OUT_OF_MEMORY;
    }

    im_shallow_copy(src, dst);
    dst->width = width;
    dst->height = height;

    dst->data = malloc(width * height * src->channels);
    if (dst->data == NULL)
    {
        return SZIM_ERROR_OUT_OF_MEMORY;
    }

    const int row_stride = src->width * src->channels;
    const int dst_row_stride = width * src->channels;
    const int channels = src->channels;

    const int xs = (int)((src->width << SHIFT) / width);
    const int ys = (int)((src->height << SHIFT) / height);

    for (int y = 0; y < height; y++)
    {
        int sy = (y * ys) >> SHIFT;
        for (int x = 0; x < width; x++)
        {
            int sx = (x * xs) >> SHIFT;

            for (int i = 0; i < channels; i++)
            {
                dst->data[y * dst_row_stride + x * channels + i] = src->data[sy * row_stride + sx * channels + i];
            }
        }
    }

    if (src->palette != NULL)
    {
        dst->palette_len = src->palette_len;
        dst->palette = malloc(dst->palette_len * 3);
        memcpy(dst->palette, src->palette, dst->palette_len * 3);
    }

    *dst_ptr = dst;
    return 0;
}

int bilinear_interp(szim_image_t *src, szim_image_t **dst_ptr, uint32_t width, uint32_t height)
{
    // TODO handle palette and trns
    int y, sy, y0, fracy;
    int x, sx, x0, fracx;
    int offset;

    szim_image_t *dst = malloc(sizeof(szim_image_t));
    if (dst == NULL)
    {
        return SZIM_ERROR_OUT_OF_MEMORY;
    }

    im_shallow_copy(src, dst);
    dst->width = width;
    dst->height = height;

    dst->data = malloc(width * height * src->channels);
    if (dst->data == NULL)
    {
        return SZIM_ERROR_OUT_OF_MEMORY;
    }

    const int row_stride = src->width * src->channels;
    const int dst_row_stride = width * src->channels;
    const int channels = src->channels;

    const int xs = (int)((double)FACTOR * src->width / width + 0.5);
    const int ys = (int)((double)FACTOR * src->height / height + 0.5);

    // Process borders first

    // Y-border
    for (y = height - 1; y >= 0; y--)
    {
        sy = y * ys;
        y0 = sy >> SHIFT;

        if (y0 < src->height - 1)
        {
            break;
        }

        fracy = sy - (y0 << SHIFT);
        for (x = 0; x < width - 1; x++)
        {
            sx = x * xs;
            x0 = sx >> SHIFT;
            fracx = sx - (x0 << SHIFT);

            offset = y0 * row_stride + x0 * channels;
            for (int i = 0; i < channels; i++)
            {
                dst->data[y * dst_row_stride + x * channels + i] = (unsigned char)((src->data[offset + i] * (FACTOR - fracx) * (FACTOR - fracy) +
                                                                                    src->data[offset + i + channels] * fracx * (FACTOR - fracy) +
                                                                                    src->data[offset + i] * (FACTOR - fracx) * fracy +
                                                                                    src->data[offset + i + channels] * fracx * fracy +
                                                                                    (FACTOR * FACTOR / 2)) >>
                                                                                   (2 * SHIFT));
            }
        }
    }

    int y_bound = y + 1;

    // X-border
    for (x = width - 1; x >= 0; x--)
    {
        sx = x * xs;
        x0 = sx >> SHIFT;

        if (x0 < src->width - 1)
        {
            break;
        }

        fracx = sx - (x0 << SHIFT);
        for (y = 0; y < height - 1; y++)
        {
            sy = y * ys;
            y0 = sy >> SHIFT;
            fracy = sy - (y0 << SHIFT);

            offset = y0 * row_stride + x0 * channels;
            for (int i = 0; i < channels; i++)
            {
                dst->data[y * dst_row_stride + x * channels + i] = (unsigned char)((src->data[offset + i] * (FACTOR - fracx) * (FACTOR - fracy) +
                                                                                    src->data[offset + i] * fracx * (FACTOR - fracy) +
                                                                                    src->data[offset + row_stride + i] * (FACTOR - fracx) * fracy +
                                                                                    src->data[offset + row_stride + i] * fracx * fracy +
                                                                                    (FACTOR * FACTOR / 2)) >>
                                                                                   (2 * SHIFT));
            }
        }
    }

    int x_bound = x + 1;

    // XY-corner
    const int corner_offset = (src->height - 1) * row_stride + (src->width - 1) * channels;
    for (y = height - 1; y >= y_bound; y--)
    {
        for (x = width - 1; x >= x_bound; x--)
        {
            for (int i = 0; i < channels; i++)
            {
                dst->data[y * dst_row_stride + x * channels + i] = (unsigned char)(src->data[corner_offset + i]);
            }
        }
    }

    // Rest of the image
    for (y = 0; y < y_bound; y++)
    {
        sy = y * ys;
        y0 = sy >> SHIFT;
        fracy = sy - (y0 << SHIFT);

        for (int x = 0; x < x_bound; x++)
        {
            sx = x * xs;
            x0 = sx >> SHIFT;
            fracx = sx - (x0 << SHIFT);

            offset = y0 * row_stride + x0 * channels;

            for (int i = 0; i < channels; i++)
            {

                dst->data[y * dst_row_stride + x * channels + i] = (unsigned char)((src->data[offset + i] * (FACTOR - fracx) * (FACTOR - fracy) +
                                                                                    src->data[offset + i + channels] * fracx * (FACTOR - fracy) +
                                                                                    src->data[offset + row_stride + i] * (FACTOR - fracx) * fracy +
                                                                                    src->data[offset + row_stride + i + channels] * fracx * fracy +
                                                                                    (FACTOR * FACTOR / 2)) >>
                                                                                   (2 * SHIFT));
            }
        }
    }

    *dst_ptr = dst;

    return 0;
}