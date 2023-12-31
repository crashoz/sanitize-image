#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>

#include <spng.h>
#include <jpeglib.h>
#include <jerror.h>

#include <sanitize-image.h>

int png_encode(const char *path, szim_image_t *image, szim_output_png_options_t options)
{
    int errorcode = 0;
    int fmt;
    int ret = 0;
    spng_ctx *ctx = NULL;
    struct spng_ihdr ihdr = {0}; /* zero-initialize to set valid defaults */
    FILE *png;

    png = fopen(path, "wb");

    if (png == NULL)
    {
        printf("error opening output file %s\n", path);
        errorcode = SZIM_ERROR_OPENING_FILE;
        goto error;
    }

    /* Creating an encoder context requires a flag */
    ctx = spng_ctx_new(SPNG_CTX_ENCODER);

    if (ctx == NULL)
    {
        printf("spng_ctx_new() failed\n");
        errorcode = SZIM_ERROR_INTERNAL;
        goto error;
    }

    /* Alternatively you can set an output FILE* or stream with spng_set_png_file() or spng_set_png_stream() */
    if (spng_set_png_file(ctx, png) != 0)
    {
        errorcode = SZIM_ERROR_SET_DEST;
        goto error;
    }

    /* Set image properties, this determines the destination image format */
    ihdr.width = image->width;
    ihdr.height = image->height;
    ihdr.color_type = color_type_to_png(image->color);
    ihdr.bit_depth = image->bit_depth;
    ihdr.filter_method = 0;
    ihdr.interlace_method = options.interlace ? 1 : 0;

    size_t length = image->width * image->height * image->channels;
    /* Valid color type, bit depth combinations: https://www.w3.org/TR/2003/REC-PNG-20031110/#table111 */

    ret = spng_set_ihdr(ctx, &ihdr);
    if (ret != 0)
    {
        printf("spng_set_ihdr() error: %s\n", spng_strerror(ret));
        errorcode = SZIM_ERROR_INTERNAL;
        goto error;
    }

    spng_set_option(ctx, SPNG_IMG_COMPRESSION_LEVEL, options.compression_level);
    spng_set_option(ctx, SPNG_FILTER_CHOICE, options.filter);

    if (image->color == SZIM_COLOR_PALETTE)
    {
        struct spng_plte plte;
        plte.n_entries = image->palette_len;
        for (int i = 0; i < image->palette_len; i++)
        {
            plte.entries[i].red = image->palette[i * 3];
            plte.entries[i].green = image->palette[i * 3 + 1];
            plte.entries[i].blue = image->palette[i * 3 + 2];
            plte.entries[i].alpha = 255;
        }
        ret = spng_set_plte(ctx, &plte);
        if (ret != 0)
        {
            printf("spng_set_plte() error: %s\n", spng_strerror(ret));
            errorcode = SZIM_ERROR_ENCODE;
            goto error;
        }
    }

    if (image->trns_len != 0)
    {
        struct spng_trns trns;

        switch (image->color)
        {
        case SZIM_COLOR_GRAY:
            trns.gray = *(uint16_t *)(image->trns);
            break;
        case SZIM_COLOR_RGB:
            trns.red = *(uint16_t *)(image->trns);
            trns.green = *((uint16_t *)(image->trns) + 1);
            trns.blue = *((uint16_t *)(image->trns) + 2);
            break;
        case SZIM_COLOR_PALETTE:
            trns.n_type3_entries = image->trns_len;
            memcpy(trns.type3_alpha, image->trns, image->trns_len);
            break;
        }

        ret = spng_set_trns(ctx, &trns);
        if (ret != 0)
        {
            printf("spng_set_trns() error: %s\n", spng_strerror(ret));
            errorcode = SZIM_ERROR_ENCODE;
            goto error;
        }
    }

    /* When encoding fmt is the source format */
    /* SPNG_FMT_PNG is a special value that matches the format in ihdr */
    fmt = SPNG_FMT_PNG;

    /* SPNG_ENCODE_FINALIZE will finalize the PNG with the end-of-file marker */
    ret = spng_encode_image(ctx, image->data, length, fmt, SPNG_ENCODE_FINALIZE);

    if (ret != 0)
    {
        printf("spng_encode_image() error: %s\n", spng_strerror(ret));
        errorcode = SZIM_ERROR_ENCODE;
        goto error;
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

error:
    fclose(png);
    spng_ctx_free(ctx);

    return errorcode;
}

struct custom_enc_error_mgr
{
    struct jpeg_error_mgr pub; /* "public" fields */

    jmp_buf setjmp_buffer; /* for return to caller */
};

typedef struct custom_enc_error_mgr *custom_enc_error_ptr;

/*
 * Here's the routine that will replace the standard error_exit method:
 */

void custom_enc_error_exit(j_common_ptr cinfo)
{
    /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
    custom_enc_error_ptr err = (custom_enc_error_ptr)cinfo->err;

    /* Always display the message. */
    /* We could postpone this until after returning, if we chose. */
    (*cinfo->err->output_message)(cinfo);

    /* Return control to the setjmp point */
    longjmp(err->setjmp_buffer, 1);
}

int jpeg_encode(const char *path, szim_image_t *image, szim_output_jpeg_options_t options)
{
    int errorcode;
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
    struct custom_enc_error_mgr jerr;
    /* More stuff */
    FILE *outfile; /* target file */
    JSAMPARRAY image_buffer = NULL;
    /* Points to large array of R,G,B-order data */
    JSAMPROW row_pointer[1]; /* pointer to JSAMPLE row[s] */
    int row_stride;          /* physical row width in image buffer */
    int row, col;
    int data_precision = 8;

    if ((outfile = fopen(path, "wb")) == NULL)
    {
        fprintf(stderr, "can't open %s\n", path);
        errorcode = SZIM_ERROR_OPENING_FILE;
        goto error;
    }

    /* Step 1: allocate and initialize JPEG compression object */

    /* We have to set up the error handler first, in case the initialization
     * step fails.  (Unlikely, but it could happen if you are out of memory.)
     * This routine fills in the contents of struct jerr, and returns jerr's
     * address which we place into the link field in cinfo.
     */

    /* We set up the normal JPEG error routines, then override error_exit. */
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = custom_enc_error_exit;
    /* Establish the setjmp return context for my_error_exit to use. */
    if (setjmp(jerr.setjmp_buffer))
    {
        /* If we get here, the JPEG code has signaled an error.
         * We need to clean up the JPEG object, close the input file, and return.
         */
        switch (jerr.pub.msg_code)
        {
        case JERR_FILE_READ:
            errorcode = SZIM_ERROR_SET_SOURCE;
            break;
        case JERR_OUT_OF_MEMORY:
            errorcode = SZIM_ERROR_OUT_OF_MEMORY;
            break;
        default:
            errorcode = SZIM_ERROR_INTERNAL;
            break;
        }

        jpeg_destroy_compress(&cinfo);
        fclose(outfile);
        return errorcode;
    }

    /* Now we can initialize the JPEG compression object. */
    jpeg_create_compress(&cinfo);

    /* Step 2: specify data destination (eg, a file) */
    /* Note: steps 2 and 3 can be done in either order. */

    /* Here we use the library-supplied code to send compressed data to a
     * stdio stream.  You can also write your own code to do something else.
     * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
     * requires it in order to write binary files.
     */
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

    cinfo.arith_code = options.arith_code;
    cinfo.dct_method = options.dct_method;
    cinfo.optimize_coding = options.optimize;
    cinfo.smoothing_factor = options.smoothing;
    /* Now use the library's routine to set default compression parameters.
     * (You must set at least cinfo.in_color_space before calling this,
     * since the defaults depend on the source color space.)
     */
    jpeg_set_defaults(&cinfo);
    /* Now you can set any non-default parameters you wish to.
     * Here we just illustrate the use of quality (quantization table) scaling:
     */
    jpeg_set_quality(&cinfo, options.quality, TRUE /* limit to baseline-JPEG values */);
    /* Use 4:4:4 subsampling (default is 4:2:0) */
    cinfo.comp_info[0].h_samp_factor = cinfo.comp_info[0].v_samp_factor = 1;

    if (options.progressive)
    {
        jpeg_simple_progression(&cinfo);
    }

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

    // ? We can skip this and write directly from image to output file (see below)
    /*
    image_buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, image->height);

    for (row = 0; row < image->height; row++)
    {
        for (col = 0; col < image->width; col++)
        {
            image_buffer[row][col * 3] = image->data[row * image->width * 3 + col * 3];
            image_buffer[row][col * 3 + 1] = image->data[row * image->width * 3 + col * 3 + 1];
            image_buffer[row][col * 3 + 2] = image->data[row * image->width * 3 + col * 3 + 2];
        }
    }
    */

    /* Step 6: while (scan lines remain to be written) */
    /*           jpeg_write_scanlines(...); */

    /* Here we use the library's state variable cinfo.next_scanline as the
     * loop counter, so that we don't have to keep track ourselves.
     * To keep things simple, we pass one scanline per call; you can pass
     * more if you wish, though.
     */

    while (cinfo.next_scanline < cinfo.image_height)
    {
        /* jpeg_write_scanlines expects an array of pointers to scanlines.
         * Here the array is only one element long, but you could pass
         * more than one scanline at a time if that's more convenient.
         */

        // row_pointer[0] = image_buffer[cinfo.next_scanline];
        row_pointer[0] = &image->data[cinfo.next_scanline * row_stride];

        (void)jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    /* Step 7: Finish compression */

    jpeg_finish_compress(&cinfo);
    /* After finish_compress, we can close the output file. */
    fclose(outfile);

    /* Step 8: release JPEG compression object */

    /* This is an important step since it will release a good deal of memory. */
    jpeg_destroy_compress(&cinfo);

    /* And we're done! */
    return 0;

error:
    jpeg_destroy_compress(&cinfo);
    fclose(outfile);
    return errorcode;
}
