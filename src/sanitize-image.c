#include <stdio.h>
#include <sanitize-image.h>

void sanitize(unsigned char *data, size_t size, const char *path)
{
    int ret;

    image_t *im;
    // ret = png_decode(data, size, 1024, 1024, 1 << 20, &im);
    ret = jpeg_decode(data, size, 1024, 1024, 1 << 20, &im);

    printf("code: %d\n", ret);
    printf("size: (%d, %d)\n", im->width, im->height);

    randomize_rgb(im);
    image_t *resized_im;
    bilinear_interp(im, &resized_im, 512, 512);

    free(im->data);
    free(im);

    // png_encode("../../new.png", resized_im, SPNG_COLOR_TYPE_TRUECOLOR, 8);
    jpeg_encode(path, resized_im, 90);

    free(resized_im->data);
    free(resized_im);
}