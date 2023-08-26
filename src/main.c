#include <sanitize-image.h>
#include <spng.h>

int main(int argc, char **argv)
{
    hello_sanitizer();

    int ret;

    image_t *im;
    // ret = png_decode("../../pusheen.png", 1024, 1024, 1 << 20, &im);
    ret = jpeg_decode("../../pusheen.jpg", 1024, 1024, 1 << 20, &im);

    printf("code: %d\n", ret);

    // randomize_rgb(im);
    image_t *resized_im;
    bilinear_interp(im, &resized_im, 400, 400);

    // png_encode("../../new.png", im, SPNG_COLOR_TYPE_TRUECOLOR, 8);
    jpeg_encode("../../new.jpg", resized_im, 90);

    free(im->data);
    free(im);
    return 0;
}