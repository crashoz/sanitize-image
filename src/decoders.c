#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include <spng.h>
#include <jpeglib.h>
#include <jerror.h>

#include <sanitize-image.h>

const char *color_type_str(enum spng_color_type color_type)
{
    switch (color_type)
    {
    case SPNG_COLOR_TYPE_GRAYSCALE:
        return "grayscale";
    case SPNG_COLOR_TYPE_TRUECOLOR:
        return "truecolor";
    case SPNG_COLOR_TYPE_INDEXED:
        return "indexed color";
    case SPNG_COLOR_TYPE_GRAYSCALE_ALPHA:
        return "grayscale with alpha";
    case SPNG_COLOR_TYPE_TRUECOLOR_ALPHA:
        return "truecolor with alpha";
    default:
        return "(invalid)";
    }
}

image_t *png_decode(char *path, uint32_t max_width, uint32_t max_height, size_t max_size)
{
    FILE *png;
    int ret = 0;
    spng_ctx *ctx = NULL;
    image_t *image = malloc(sizeof(image));

    png = fopen(path, "rb");

    if (png == NULL)
    {
        printf("error opening input file %s\n", path);
        goto error;
    }

    ctx = spng_ctx_new(0);

    if (ctx == NULL)
    {
        printf("spng_ctx_new() failed\n");
        goto error;
    }

    /* Ignore and don't calculate chunk CRC's */
    spng_set_crc_action(ctx, SPNG_CRC_USE, SPNG_CRC_USE);

    //! limits against malicious input
    spng_set_image_limits(ctx, max_width, max_height);

    /* Set memory usage limits for storing standard and unknown chunks,
       this is important when reading untrusted files! */
    size_t limit = 1024 * 1024 * 64;
    spng_set_chunk_limits(ctx, limit, limit);

    /* Set source PNG */
    spng_set_png_file(ctx, png); /* or _buffer(), _stream() */

    struct spng_ihdr ihdr;
    ret = spng_get_ihdr(ctx, &ihdr);

    if (ret)
    {
        printf("spng_get_ihdr() error: %s\n", spng_strerror(ret));
        goto error;
    }

    const char *color_name = color_type_str(ihdr.color_type);

    image->width = ihdr.width;
    image->height = ihdr.height;

    printf("width: %u\n"
           "height: %u\n"
           "bit depth: %u\n"
           "color type: %u - %s\n",
           ihdr.width, ihdr.height, ihdr.bit_depth, ihdr.color_type, color_name);

    printf("compression method: %u\n"
           "filter method: %u\n"
           "interlace method: %u\n",
           ihdr.compression_method, ihdr.filter_method, ihdr.interlace_method);

    struct spng_plte plte = {0};
    ret = spng_get_plte(ctx, &plte);

    if (ret && ret != SPNG_ECHUNKAVAIL)
    {
        printf("spng_get_plte() error: %s\n", spng_strerror(ret));
        goto error;
    }

    if (!ret)
        printf("palette entries: %u\n", plte.n_entries);

    size_t image_size, image_width;

    /* Output format, does not depend on source PNG format except for
       SPNG_FMT_PNG, which is the PNG's format in host-endian or
       big-endian for SPNG_FMT_RAW.
       Note that for these two formats <8-bit images are left byte-packed */
    int fmt = SPNG_FMT_PNG;

    /* With SPNG_FMT_PNG indexed color images are output as palette indices,
       pick another format to expand them. */
    if (ihdr.color_type == SPNG_COLOR_TYPE_INDEXED)
        fmt = SPNG_FMT_RGB8;

    ret = spng_decoded_image_size(ctx, fmt, &image_size);

    if (ret)
        goto error;

    if (image_size > max_size)
    {
        printf("image is too big: %d\n", image_size);
        goto error;
    }

    printf("image size: %u\n", image_size);

    image->data = malloc(image_size);

    if (image->data == NULL)
        goto error;

    /* Decode the image in one go */
    /* ret = spng_decode_image(ctx, image, image_size, SPNG_FMT_RGBA8, 0);

    if(ret)
    {
        printf("spng_decode_image() error: %s\n", spng_strerror(ret));
        goto error;
    }*/

    /* Alternatively you can decode the image progressively,
       this requires an initialization step. */
    ret = spng_decode_image(ctx, NULL, 0, fmt, SPNG_DECODE_PROGRESSIVE);

    if (ret)
    {
        printf("progressive spng_decode_image() error: %s\n", spng_strerror(ret));
        goto error;
    }

    /* ihdr.height will always be non-zero if spng_get_ihdr() succeeds */
    image_width = image_size / ihdr.height;

    struct spng_row_info row_info = {0};

    do
    {
        ret = spng_get_row_info(ctx, &row_info);
        if (ret)
            break;

        ret = spng_decode_row(ctx, image->data + row_info.row_num * image_width, image_width);
    } while (!ret);

    if (ret != SPNG_EOI)
    {
        printf("progressive decode error: %s\n", spng_strerror(ret));

        if (ihdr.interlace_method)
            printf("last pass: %d, scanline: %u\n", row_info.pass, row_info.scanline_idx);
        else
            printf("last row: %u\n", row_info.row_num);
    }

    /*
    ? parse text contained in the png data
    uint32_t n_text = 0;
    struct spng_text *text = NULL;

    ret = spng_get_text(ctx, NULL, &n_text);

    if (ret == SPNG_ECHUNKAVAIL) // No text chunks in file
    {
        ret = 0;
        goto no_text;
    }

    if (ret)
    {
        printf("spng_get_text() error: %s\n", spng_strerror(ret));
        goto error;
    }

    text = malloc(n_text * sizeof(struct spng_text));

    if (text == NULL)
        goto error;

    ret = spng_get_text(ctx, text, &n_text);

    if (ret)
    {
        printf("spng_get_text() error: %s\n", spng_strerror(ret));
        goto no_text;
    }

    uint32_t i;
    for (i = 0; i < n_text; i++)
    {
        char *type_str = "tEXt";
        if (text[i].type == SPNG_ITXT)
            type_str = "iTXt";
        else if (text[i].type == SPNG_ZTXT)
            type_str = "zTXt";

        printf("\ntext type: %s\n", type_str);
        printf("keyword: %s\n", text[i].keyword);

        if (text[i].type == SPNG_ITXT)
        {
            printf("language tag: %s\n", text[i].language_tag);
            printf("translated keyword: %s\n", text[i].translated_keyword);
        }

        printf("text is %scompressed\n", text[i].compression_flag ? "" : "not ");
        printf("text length: %lu\n", (unsigned long int)text[i].length);
        printf("text: %s\n", text[i].text);
    }

no_text:
    free(text);

    */

    return image;

error:

    spng_ctx_free(ctx);
    free(image->data);
    free(image);

    return NULL;
}

image_t *jpeg_decode(char *path, uint32_t max_width, uint32_t max_height, size_t max_size)
{
    // TODO Force image output format
    // TODO Handle errors and malicious input

    struct jpeg_decompress_struct internal_cinfo;
    struct jpeg_decompress_struct *cinfo = &internal_cinfo;
    struct jpeg_error_mgr jerr;
    FILE *infile;                 /* source file */
    JSAMPARRAY buffer = NULL;     /* Output row buffer */
    J12SAMPARRAY buffer12 = NULL; /* 12-bit output row buffer */
    int col;
    int row_stride; /* physical row width in output buffer */

    image_t *image = malloc(sizeof(image));

    /* In this example we want to open the input and output files before doing
     * anything else, so that the setjmp() error recovery below can assume the
     * files are open.
     *
     * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
     * requires it in order to read/write binary files.
     */

    if ((infile = fopen(path, "rb")) == NULL)
    {
        fprintf(stderr, "can't open %s\n", path);
        return 0;
    }

    cinfo->err = jpeg_std_error(&jerr);

    /* Step 1: allocate and initialize JPEG decompression object */

    /* Now we can initialize the JPEG decompression object. */
    jpeg_create_decompress(cinfo);

    /* Step 2: specify data source (eg, a file) */

    jpeg_stdio_src(cinfo, infile);

    /* Step 3: read file parameters with jpeg_read_header() */

    (void)jpeg_read_header(cinfo, TRUE);
    /* We can ignore the return value from jpeg_read_header since
     *   (a) suspension is not possible with the stdio data source, and
     *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
     * See libjpeg.txt for more info.
     */

    //! limits against malicious input
    if (cinfo->image_width > max_width || cinfo->image_height > max_height)
    {
        // TODO handle image dims are too big
        return NULL;
    }

    if ((cinfo->image_width * cinfo->image_height * cinfo->num_components * cinfo->data_precision) / 8 > max_size)
    {
        // TODO handle image is too big
        return NULL;
    }

    image->width = cinfo->image_width;
    image->height = cinfo->image_height;

    /* Step 4: set parameters for decompression */

    /* In this example, we don't need to change any of the defaults set by
     * jpeg_read_header(), so we do nothing here.
     */

    /* Step 5: Start decompressor */

    (void)jpeg_start_decompress(cinfo);
    /* We can ignore the return value since suspension is not possible
     * with the stdio data source.
     */

    /* We may need to do some setup of our own at this point before reading
     * the data.  After jpeg_start_decompress() we have the correct scaled
     * output image dimensions available, as well as the output colormap
     * if we asked for color quantization.
     * In this example, we need to make an output work buffer of the right size.
     */
    /* Samples per row in output buffer */
    row_stride = cinfo->output_width * cinfo->output_components;
    /* Make a one-row-high sample array that will go away when done with image */
    if (cinfo->data_precision == 12)
    {
        buffer12 = (J12SAMPARRAY)(*cinfo->mem->alloc_sarray)((j_common_ptr)cinfo, JPOOL_IMAGE, row_stride, 1);
    }
    else
    {
        buffer = (*cinfo->mem->alloc_sarray)((j_common_ptr)cinfo, JPOOL_IMAGE, row_stride, 1);
        image->data = malloc(image->height * row_stride);
    }

    /* Step 6: while (scan lines remain to be read) */
    /*           jpeg_read_scanlines(...); */

    /* Here we use the library's state variable cinfo->output_scanline as the
     * loop counter, so that we don't have to keep track ourselves.
     */
    if (cinfo->data_precision == 12)
    {
        while (cinfo->output_scanline < cinfo->output_height)
        {
            /* jpeg12_read_scanlines expects an array of pointers to scanlines.
             * Here the array is only one element long, but you could ask for
             * more than one scanline at a time if that's more convenient.
             */
            (void)jpeg12_read_scanlines(cinfo, buffer12, 1);
            /* Swap MSB and LSB in each sample */
            for (col = 0; col < row_stride; col++)
                buffer12[0][col] = ((buffer12[0][col] & 0xFF) << 8) |
                                   ((buffer12[0][col] >> 8) & 0xFF);

            // TODO Handle 12-bit images
            // fwrite(buffer12[0], 1, row_stride * sizeof(J12SAMPLE), outfile);
        }
    }
    else
    {
        while (cinfo->output_scanline < cinfo->output_height)
        {
            /* jpeg_read_scanlines expects an array of pointers to scanlines.
             * Here the array is only one element long, but you could ask for
             * more than one scanline at a time if that's more convenient.
             */
            (void)jpeg_read_scanlines(cinfo, buffer, 1);
            memcpy(&(image->data[cinfo->output_scanline * row_stride]), buffer[0], row_stride);
            // fwrite(buffer[0], 1, row_stride, outfile);
        }
    }

    /* Step 7: Finish decompression */

    (void)jpeg_finish_decompress(cinfo);
    /* We can ignore the return value since suspension is not possible
     * with the stdio data source.
     */

    /* Step 8: Release JPEG decompression object */

    /* This is an important step since it will release a good deal of memory. */
    jpeg_destroy_decompress(cinfo);

    /* After finish_decompress, we can close the input and output files.
     * Here we postpone it until after no more JPEG errors are possible,
     * so as to simplify the setjmp error logic above.  (Actually, I don't
     * think that jpeg_destroy can do an error exit, but why assume anything...)
     */
    fclose(infile);
    // fclose(outfile);

    /* At this point you may want to check to see whether any corrupt-data
     * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
     */

    /* And we're done! */
    return image;
}