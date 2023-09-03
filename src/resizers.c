#include <sanitize-image.h>

#define FACTOR 2048
#define SHIFT 11

int resize(image_t *src, image_t **dst_ptr, uint32_t width, uint32_t height, resizer_type type)
{
    if (type == RESIZER_NONE)
    {
        *dst_ptr = src;
        return 0;
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
    case RESIZER_BILINEAR:
        bilinear_interp(src, dst_ptr, width, height);
        break;
    default:
        break;
    }

    return 0;
}

int bilinear_interp(image_t *src, image_t **dst_ptr, uint32_t width, uint32_t height)
{
    int y, sy, y0, fracy;
    int x, sx, x0, fracx;
    int offset;

    image_t *dst = malloc(sizeof(image_t));
    dst->data = malloc(width * height * 3);
    dst->width = width;
    dst->height = height;

    const int row_stride = src->width * 3;

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

            offset = y0 * row_stride + x0 * 3;
            for (int i = 0; i < 3; i++)
            {
                dst->data[y * (width * 3) + x * 3 + i] = (unsigned char)((src->data[offset + i] * (FACTOR - fracx) * (FACTOR - fracy) +
                                                                          src->data[offset + i + 3] * fracx * (FACTOR - fracy) +
                                                                          src->data[offset + i] * (FACTOR - fracx) * fracy +
                                                                          src->data[offset + i + 3] * fracx * fracy +
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

            offset = y0 * row_stride + x0 * 3;
            for (int i = 0; i < 3; i++)
            {
                dst->data[y * (width * 3) + x * 3 + i] = (unsigned char)((src->data[offset + i] * (FACTOR - fracx) * (FACTOR - fracy) +
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
    const int corner_offset = (src->height - 1) * row_stride + (src->width - 1) * 3;
    for (y = height - 1; y >= y_bound; y--)
    {
        for (x = width - 1; x >= x_bound; x--)
        {
            for (int i = 0; i < 3; i++)
            {
                dst->data[y * (width * 3) + x * 3 + i] = (unsigned char)(src->data[corner_offset + i]);
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

            offset = y0 * row_stride + x0 * 3;

            for (int i = 0; i < 3; i++)
            {
                dst->data[y * (width * 3) + x * 3 + i] = (unsigned char)((src->data[offset + i] * (FACTOR - fracx) * (FACTOR - fracy) +
                                                                          src->data[offset + i + 3] * fracx * (FACTOR - fracy) +
                                                                          src->data[offset + row_stride + i] * (FACTOR - fracx) * fracy +
                                                                          src->data[offset + row_stride + i + 3] * fracx * fracy +
                                                                          (FACTOR * FACTOR / 2)) >>
                                                                         (2 * SHIFT));
            }
        }
    }

    *dst_ptr = dst;

    return 0;
}