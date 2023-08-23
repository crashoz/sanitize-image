#include <sanitize-image.h>
#include <spng.h>

int main(int argc, char **argv)
{
    hello_sanitizer();

    image_t *im = png_decode("../../pusheen.png", 1024, 1024, 1 << 20);

    randomize_rgb(im);

    // png_encode("../../new.png", im, SPNG_COLOR_TYPE_TRUECOLOR, 8);
    jpeg_encode("../../new.jpg", im, 90, 8);

    free(im->data);
    free(im);
    return 0;
}