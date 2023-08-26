#include <sanitize-image.h>

#define FACTOR 2048
#define SHIFT 11

int bilinear_interp(image_t *src, image_t **dst_ptr, uint32_t width, uint32_t height)
{
    image_t *dst = malloc(sizeof(image_t));
    dst->data = malloc(width * height * 3);
    dst->width = width;
    dst->height = height;

    const int row_stride = src->width * 3;

    const int xs = (int)((double)FACTOR * src->width / width + 0.5);
    const int ys = (int)((double)FACTOR * src->height / height + 0.5);

    for (int y = 0; y < height; y++)
    {
        const int sy = y * ys;
        const int y0 = sy >> SHIFT;
        const int fracy = sy - (y0 << SHIFT);

        for (int x = 0; x < width; x++)
        {
            const int sx = x * xs;
            const int x0 = sx >> SHIFT;
            const int fracx = sx - (x0 << SHIFT);

            if (x0 >= src->width - 1 || y0 >= src->height - 1)
            {
                // insert special handling here
                continue;
            }

            const int offset = y0 * row_stride + x0 * 3;

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