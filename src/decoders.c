#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>

#include <spng.h>
#include <jpeglib.h>
#include <jerror.h>

#include <sanitize-image.h>

int png_decode(unsigned char *buffer, size_t buffer_size, uint32_t max_width, uint32_t max_height, size_t max_size, image_t **out_image)
{
    int errorcode = 0;
    FILE *png;
    int ret = 0;
    spng_ctx *ctx = NULL;

    *out_image = NULL;

    image_t *image = malloc(sizeof(image_t));
    if (image == NULL)
    {
        errorcode = ERROR_OUT_OF_MEMORY;
        goto error;
    }
    image->data = NULL;

    ctx = spng_ctx_new(0);

    if (ctx == NULL)
    {
        printf("spng_ctx_new() failed\n");
        errorcode = ERROR_INTERNAL;
        goto error;
    }

    /* Ignore and don't calculate chunk CRC's */
    if (spng_set_crc_action(ctx, SPNG_CRC_USE, SPNG_CRC_USE) != 0)
    {
        errorcode = ERROR_INTERNAL;
        goto error;
    }

    //! limits against malicious input
    if (spng_set_image_limits(ctx, max_width, max_height) != 0)
    {
        errorcode = ERROR_INTERNAL;
        goto error;
    }

    /* Set memory usage limits for storing standard and unknown chunks,
       this is important when reading untrusted files! */
    size_t limit = 1024 * 1024 * 64;
    if (spng_set_chunk_limits(ctx, limit, limit) != 0)
    {
        errorcode = ERROR_INTERNAL;
        goto error;
    };

    /* Set source PNG */
    if (spng_set_png_buffer(ctx, buffer, buffer_size) != 0)
    {
        errorcode = ERROR_SET_SOURCE;
        goto error;
    }; /* or _buffer(), _stream() */

    struct spng_ihdr ihdr;
    ret = spng_get_ihdr(ctx, &ihdr);

    if (ret != 0)
    {
        errorcode = ERROR_BAD_HEADER;
        goto error;
    }

    image->width = ihdr.width;
    image->height = ihdr.height;

    image->bit_depth = ihdr.bit_depth;
    image->color = png_to_color_type(ihdr.color_type);

    /*
    ? display png header info
    const char *color_name = color_type_str(ihdr.color_type);

    printf("width: %u\n"
           "height: %u\n"
           "bit depth: %u\n"
           "color type: %u - %s\n",
           ihdr.width, ihdr.height, ihdr.bit_depth, ihdr.color_type, color_name);

    printf("compression method: %u\n"
           "filter method: %u\n"
           "interlace method: %u\n",
           ihdr.compression_method, ihdr.filter_method, ihdr.interlace_method);

    */

    if (image->color = COLOR_PALETTE)
    {
        struct spng_plte plte = {0};
        ret = spng_get_plte(ctx, &plte);

        if (ret != 0)
        {
            errorcode = ERROR_MISSING_PALETTE;
            goto error;
        }

        image->palette_len = plte.n_entries;

        image->palette = malloc(3 * plte.n_entries);
        if (image->palette == NULL)
        {
            errorcode = ERROR_OUT_OF_MEMORY;
            goto error;
        }

        memcpy(image->palette, plte.entries, 3 * plte.n_entries);
    }

    size_t image_size,
        image_width;
    int fmt = SPNG_FMT_RGB8;

    ret = spng_decoded_image_size(ctx, fmt, &image_size);

    if (ret != 0)
    {
        errorcode = ERROR_IMAGE_SIZE;
        goto error;
    }

    if (image_size > max_size)
    {
        printf("image is too big: %d\n", image_size);
        errorcode = ERROR_IMAGE_SIZE;
        goto error;
    }

    image->data = malloc(image_size);

    if (image->data == NULL)
    {
        errorcode = ERROR_OUT_OF_MEMORY;
        goto error;
    }

    /* Decode the image in one go */
    ret = spng_decode_image(ctx, image->data, image_size, fmt, 0);

    if (ret != 0)
    {
        printf("spng_decode_image() error: %s\n", spng_strerror(ret));
        errorcode = ERROR_DECODE;
        goto error;
    }

    /* Alternatively you can decode the image progressively,
       this requires an initialization step. */
    /*
        ret = spng_decode_image(ctx, NULL, 0, fmt, SPNG_DECODE_PROGRESSIVE);

        if (ret)
        {
            printf("progressive spng_decode_image() error: %s\n", spng_strerror(ret));
            goto error;
        }

        // ihdr.height will always be non-zero if spng_get_ihdr() succeeds
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
        */

    // Parse transparency chunk (tRNS)
    struct spng_trns trns;
    ret = spng_get_trns(ctx, &trns);

    if (ret == 0)
    {
        switch (image->color)
        {
        case COLOR_GRAYSCALE:
            image->trns = malloc(2);
            if (image->trns == NULL)
            {
                errorcode = ERROR_OUT_OF_MEMORY;
                goto error;
            }
            *(uint16_t *)(image->trns) = trns.gray;
            break;
        case COLOR_RGB:
            image->trns = malloc(3 * 2);
            if (image->trns == NULL)
            {
                errorcode = ERROR_OUT_OF_MEMORY;
                goto error;
            }
            *(uint16_t *)(image->trns) = trns.red;
            *(uint16_t *)(image->trns) = trns.green;
            *(uint16_t *)(image->trns) = trns.blue;
        case COLOR_PALETTE:
            if (image->trns == NULL)
            {
                errorcode = ERROR_OUT_OF_MEMORY;
                goto error;
            }
            image->trns_len = trns.n_type3_entries;
            image->trns = malloc(trns.n_type3_entries);
            memcpy(image->trns, trns.type3_alpha, trns.n_type3_entries);
        }
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

    *out_image = image;
    spng_ctx_free(ctx);

    return 0;

error:
    spng_ctx_free(ctx);
    if (image != NULL)
    {
        free(image->data);
    }
    free(image);

    return errorcode;
}

struct custom_dec_error_mgr
{
    struct jpeg_error_mgr pub; /* "public" fields */

    jmp_buf setjmp_buffer; /* for return to caller */
};

typedef struct custom_dec_error_mgr *custom_dec_error_ptr;

/*
 * Here's the routine that will replace the standard error_exit method:
 */

void custom_dec_error_exit(j_common_ptr cinfo)
{
    /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
    custom_dec_error_ptr err = (custom_dec_error_ptr)cinfo->err;

    /* Always display the message. */
    /* We could postpone this until after returning, if we chose. */
    (*cinfo->err->output_message)(cinfo);

    /* Return control to the setjmp point */
    longjmp(err->setjmp_buffer, 1);
}

int jpeg_decode(unsigned char *buffer, size_t buffer_size, uint32_t max_width, uint32_t max_height, size_t max_size, image_t **out_image)
{
    // TODO Handle errors and malicious input
    int errorcode = 0;
    struct jpeg_decompress_struct cinfo;
    struct custom_dec_error_mgr jerr;
    FILE *infile;                 /* source file */
    JSAMPARRAY out_buffer = NULL; /* Output row buffer */
    int col;
    int row_stride; /* physical row width in output buffer */

    image_t *image = malloc(sizeof(image_t));

    if (image == NULL)
    {
        errorcode = ERROR_OUT_OF_MEMORY;
        goto error;
    }

    image->color = COLOR_RGB;
    image->bit_depth = 8;
    image->trns = NULL;
    image->palette = NULL;

    /* In this example we want to open the input and output files before doing
     * anything else, so that the setjmp() error recovery below can assume the
     * files are open.
     *
     * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
     * requires it in order to read/write binary files.
     */

    cinfo.err = jpeg_std_error(&jerr.pub);

    /* Step 1: allocate and initialize JPEG decompression object */

    /* We set up the normal JPEG error routines, then override error_exit. */
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = custom_dec_error_exit;
    /* Establish the setjmp return context for my_error_exit to use. */
    if (setjmp(jerr.setjmp_buffer))
    {
        /* If we get here, the JPEG code has signaled an error.
         * We need to clean up the JPEG object, close the input file, and return.
         */
        switch (jerr.pub.msg_code)
        {
        case JERR_FILE_READ:
            errorcode = ERROR_SET_SOURCE;
            break;
        case JERR_OUT_OF_MEMORY:
            errorcode = ERROR_OUT_OF_MEMORY;
            break;
        default:
            errorcode = ERROR_INTERNAL;
            break;
        }

        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        return errorcode;
    }

    /* Now we can initialize the JPEG decompression object. */
    jpeg_create_decompress(&cinfo);

    /* Step 2: specify data source (eg, a file) */

    // jpeg_stdio_src(&cinfo, infile);
    jpeg_mem_src(&cinfo, buffer, buffer_size);

    /* Step 3: read file parameters with jpeg_read_header() */

    (void)jpeg_read_header(&cinfo, TRUE);
    /* We can ignore the return value from jpeg_read_header since
     *   (a) suspension is not possible with the stdio data source, and
     *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
     * See libjpeg.txt for more info.
     */

    //! limits against malicious input
    if (cinfo.image_width > max_width || cinfo.image_height > max_height)
    {
        errorcode = ERROR_IMAGE_SIZE;
        goto error;
    }

    if ((cinfo.image_width * cinfo.image_height * cinfo.num_components * cinfo.data_precision) / 8 > max_size)
    {
        errorcode = ERROR_IMAGE_SIZE;
        goto error;
    }

    image->width = cinfo.image_width;
    image->height = cinfo.image_height;

    /* Step 4: set parameters for decompression */

    cinfo.out_color_space = JCS_RGB;

    /* Step 5: Start decompressor */

    (void)jpeg_start_decompress(&cinfo);
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
    row_stride = cinfo.output_width * cinfo.output_components;
    /* Make a one-row-high sample array that will go away when done with image */
    if (cinfo.data_precision == 12)
    {
        // TODO error: does not support 12 bit data precision
        errorcode = ERROR_NOT_SUPPORTED;
        goto error;
    }
    else
    {
        out_buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);
        image->data = malloc(image->height * row_stride);
    }

    /* Step 6: while (scan lines remain to be read) */
    /*           jpeg_read_scanlines(...); */

    /* Here we use the library's state variable cinfo.output_scanline as the
     * loop counter, so that we don't have to keep track ourselves.
     */
    while (cinfo.output_scanline < cinfo.output_height)
    {
        /* jpeg_read_scanlines expects an array of pointers to scanlines.
         * Here the array is only one element long, but you could ask for
         * more than one scanline at a time if that's more convenient.
         */
        (void)jpeg_read_scanlines(&cinfo, out_buffer, 1);

        //! cinfo.output_scanline starts at 1
        memcpy(&(image->data[(cinfo.output_scanline - 1) * row_stride]), out_buffer[0], row_stride);
        // fwrite(out_buffer[0], 1, row_stride, outfile);
    }

    /* Step 7: Finish decompression */

    (void)jpeg_finish_decompress(&cinfo);
    /* We can ignore the return value since suspension is not possible
     * with the stdio data source.
     */

    /* Step 8: Release JPEG decompression object */

    /* This is an important step since it will release a good deal of memory. */
    jpeg_destroy_decompress(&cinfo);

    /* After finish_decompress, we can close the input and output files.
     * Here we postpone it until after no more JPEG errors are possible,
     * so as to simplify the setjmp error logic above.  (Actually, I don't
     * think that jpeg_destroy can do an error exit, but why assume anything...)
     */
    // fclose(infile);
    // fclose(outfile);

    /* At this point you may want to check to see whether any corrupt-data
     * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
     */

    /* And we're done! */
    *out_image = image;
    return 0;

error:
    jpeg_destroy_decompress(&cinfo);
    return errorcode;
}