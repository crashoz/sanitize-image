#include <sanitize-image.h>
#include <spng.h>

#include <inttypes.h>
#include <stdio.h>

int png_encode(char *path, image_t *image, enum spng_color_type color_type, int bit_depth)
{
    int fmt;
    int ret = 0;
    spng_ctx *ctx = NULL;
    struct spng_ihdr ihdr = {0}; /* zero-initialize to set valid defaults */
    FILE *png;

    png = fopen(path, "wb");

    if (png == NULL)
    {
        printf("error opening output file %s\n", path);
        goto encode_error;
    }

    /* Creating an encoder context requires a flag */
    ctx = spng_ctx_new(SPNG_CTX_ENCODER);

    /* Encode to internal buffer managed by the library */
    // spng_set_option(ctx, SPNG_ENCODE_TO_BUFFER, 1);
    // TODO set compression and options

    /* Alternatively you can set an output FILE* or stream with spng_set_png_file() or spng_set_png_stream() */
    spng_set_png_file(ctx, png);

    /* Set image properties, this determines the destination image format */
    ihdr.width = image->width;
    ihdr.height = image->height;
    ihdr.color_type = color_type;
    ihdr.bit_depth = bit_depth;
    size_t length = image->width * image->height * 3;
    /* Valid color type, bit depth combinations: https://www.w3.org/TR/2003/REC-PNG-20031110/#table111 */

    ret = spng_set_ihdr(ctx, &ihdr);
    if (ret)
    {
        printf("spng_set_ihdr() error: %s\n", spng_strerror(ret));
        goto encode_error;
    }

    /* When encoding fmt is the source format */
    /* SPNG_FMT_PNG is a special value that matches the format in ihdr */
    fmt = SPNG_FMT_PNG;

    /* SPNG_ENCODE_FINALIZE will finalize the PNG with the end-of-file marker */
    ret = spng_encode_image(ctx, image->data, length, fmt, SPNG_ENCODE_FINALIZE);

    if (ret)
    {
        printf("spng_encode_image() error: %s\n", spng_strerror(ret));
        goto encode_error;
    }

    /*
    ? encode to a buffer
    size_t png_size;
    void *png_buf = NULL;

    // Get the internal buffer of the finished PNG
    png_buf = spng_get_png_buffer(ctx, &png_size, &ret);

    if (png_buf == NULL)
    {
        printf("spng_get_png_buffer() error: %s\n", spng_strerror(ret));
    }

    // User owns the buffer after a successful call
    free(png_buf);
    */

encode_error:

    spng_ctx_free(ctx);

    return ret;
}