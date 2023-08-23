#include <inttypes.h>
#include <stdio.h>

#include <spng.h>
#include <jpeglib.h>
#include <jerror.h>

#include <sanitize-image.h>

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

int jpeg_encode(char *path, image_t *image, int quality, int data_precision)
{
    /* This struct contains the JPEG compression parameters and pointers to
     * working space (which is allocated as needed by the JPEG library).
     * It is possible to have several such structures, representing multiple
     * compression/decompression processes, in existence at once.  We refer
     * to any one struct (and its associated working data) as a "JPEG object".
     */
    struct jpeg_compress_struct cinfo;
    /* This struct represents a JPEG error handler.  It is declared separately
     * because applications often want to supply a specialized error handler
     * (see the second half of this file for an example).  But here we just
     * take the easy way out and use the standard error handler, which will
     * print a message on stderr and call exit() if compression fails.
     * Note that this struct must live as long as the main JPEG parameter
     * struct, to avoid dangling-pointer problems.
     */
    struct jpeg_error_mgr jerr;
    /* More stuff */
    FILE *outfile; /* target file */
    JSAMPARRAY image_buffer = NULL;
    /* Points to large array of R,G,B-order data */
    JSAMPROW row_pointer[1]; /* pointer to JSAMPLE row[s] */

    J12SAMPARRAY image_buffer12 = NULL;
    /* Points to large array of R,G,B-order 12-bit
       data */
    J12SAMPROW row_pointer12[1]; /* pointer to J12SAMPLE row[s] */
    int row_stride;              /* physical row width in image buffer */
    int row, col;

    /* Step 1: allocate and initialize JPEG compression object */

    /* We have to set up the error handler first, in case the initialization
     * step fails.  (Unlikely, but it could happen if you are out of memory.)
     * This routine fills in the contents of struct jerr, and returns jerr's
     * address which we place into the link field in cinfo.
     */
    cinfo.err = jpeg_std_error(&jerr);
    /* Now we can initialize the JPEG compression object. */
    jpeg_create_compress(&cinfo);

    /* Step 2: specify data destination (eg, a file) */
    /* Note: steps 2 and 3 can be done in either order. */

    /* Here we use the library-supplied code to send compressed data to a
     * stdio stream.  You can also write your own code to do something else.
     * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
     * requires it in order to write binary files.
     */
    if ((outfile = fopen(path, "wb")) == NULL)
        ERREXIT(&cinfo, JERR_FILE_WRITE);
    jpeg_stdio_dest(&cinfo, outfile);

    /* Step 3: set parameters for compression */

    /* First we supply a description of the input image.
     * Four fields of the cinfo struct must be filled in:
     */
    cinfo.image_width = image->width; /* image width and height, in pixels */
    cinfo.image_height = image->height;
    cinfo.input_components = 3;            /* # of color components per pixel */
    cinfo.in_color_space = JCS_RGB;        /* colorspace of input image */
    cinfo.data_precision = data_precision; /* data precision of input image */
    /* Now use the library's routine to set default compression parameters.
     * (You must set at least cinfo.in_color_space before calling this,
     * since the defaults depend on the source color space.)
     */
    jpeg_set_defaults(&cinfo);
    /* Now you can set any non-default parameters you wish to.
     * Here we just illustrate the use of quality (quantization table) scaling:
     */
    jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);
    /* Use 4:4:4 subsampling (default is 4:2:0) */
    cinfo.comp_info[0].h_samp_factor = cinfo.comp_info[0].v_samp_factor = 1;

    /* Step 4: Start compressor */

    /* TRUE ensures that we will write a complete interchange-JPEG file.
     * Pass TRUE unless you are very sure of what you're doing.
     */
    jpeg_start_compress(&cinfo, TRUE);

    /* Step 5: allocate and initialize image buffer */

    row_stride = image->width * 3; /* J[12]SAMPLEs per row in image_buffer */
    /* Make a sample array that will go away when done with image.  Note that,
     * for the purposes of this example, we could also create a one-row-high
     * sample array and initialize it for each successive scanline written in the
     * scanline loop below.
     */
    if (cinfo.data_precision == 12)
    {
        image_buffer12 = (J12SAMPARRAY)(*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, image->height);

        /* Initialize image buffer with a repeating pattern */
        for (row = 0; row < image->height; row++)
        {
            for (col = 0; col < image->width; col++)
            {
                image_buffer12[row][col * 3] =
                    (col * (MAXJ12SAMPLE + 1) / image->width) % (MAXJ12SAMPLE + 1);
                image_buffer12[row][col * 3 + 1] =
                    (row * (MAXJ12SAMPLE + 1) / image->height) % (MAXJ12SAMPLE + 1);
                image_buffer12[row][col * 3 + 2] =
                    (row * (MAXJ12SAMPLE + 1) / image->height +
                     col * (MAXJ12SAMPLE + 1) / image->width) %
                    (MAXJ12SAMPLE + 1);
            }
        }
    }
    else
    {
        image_buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, image->height);

        for (row = 0; row < image->height; row++)
        {
            for (col = 0; col < image->width; col++)
            {
                image_buffer[row][col * 3] =
                    (col * (MAXJSAMPLE + 1) / image->width) % (MAXJSAMPLE + 1);
                image_buffer[row][col * 3 + 1] =
                    (row * (MAXJSAMPLE + 1) / image->height) % (MAXJSAMPLE + 1);
                image_buffer[row][col * 3 + 2] =
                    (row * (MAXJSAMPLE + 1) / image->height + col * (MAXJSAMPLE + 1) / image->width) %
                    (MAXJSAMPLE + 1);
            }
        }
    }

    /* Step 6: while (scan lines remain to be written) */
    /*           jpeg_write_scanlines(...); */

    /* Here we use the library's state variable cinfo.next_scanline as the
     * loop counter, so that we don't have to keep track ourselves.
     * To keep things simple, we pass one scanline per call; you can pass
     * more if you wish, though.
     */
    if (cinfo.data_precision == 12)
    {
        while (cinfo.next_scanline < cinfo.image_height)
        {
            /* jpeg12_write_scanlines expects an array of pointers to scanlines.
             * Here the array is only one element long, but you could pass
             * more than one scanline at a time if that's more convenient.
             */
            row_pointer12[0] = image_buffer12[cinfo.next_scanline];
            (void)jpeg12_write_scanlines(&cinfo, row_pointer12, 1);
        }
    }
    else
    {
        while (cinfo.next_scanline < cinfo.image_height)
        {
            /* jpeg_write_scanlines expects an array of pointers to scanlines.
             * Here the array is only one element long, but you could pass
             * more than one scanline at a time if that's more convenient.
             */
            row_pointer[0] = image_buffer[cinfo.next_scanline];
            (void)jpeg_write_scanlines(&cinfo, row_pointer, 1);
        }
    }

    /* Step 7: Finish compression */

    jpeg_finish_compress(&cinfo);
    /* After finish_compress, we can close the output file. */
    fclose(outfile);

    /* Step 8: release JPEG compression object */

    /* This is an important step since it will release a good deal of memory. */
    jpeg_destroy_compress(&cinfo);

    /* And we're done! */
}
