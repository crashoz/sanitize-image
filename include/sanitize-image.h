#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <spng.h>
#include <jpeglib.h>

#define MAX_ALLOWED_TYPES 8

typedef enum
{
    TYPE_UNKNOWN,
    TYPE_INPUT,
    TYPE_PNG,
    TYPE_JPEG
} image_type;

typedef enum
{
    COLOR_UNKNOWN,
    COLOR_INPUT,
    COLOR_GRAY,
    COLOR_GRAYA,
    COLOR_RGB,
    COLOR_RGBA,
    COLOR_PALETTE
} color_type;

typedef struct
{
    image_type allowed_types[MAX_ALLOWED_TYPES];
    uint32_t max_width;
    uint32_t max_height;
    size_t max_size;
} input_options_t;

typedef enum
{
    RANDOMIZER_NONE,
    RANDOMIZER_AUTO
} randomizer_type;

typedef struct
{
    randomizer_type type;
} randomizer_options_t;
typedef enum
{
    RESIZER_NONE,
    RESIZER_BILINEAR
} resizer_type;
typedef struct
{
    resizer_type type;
    uint32_t width;
    uint32_t height;
} resizer_options_t;

typedef struct
{
    color_type color_type;
    int compression_level;
    enum spng_filter_choice filter;
    bool interlace;
} output_png_options_t;

typedef struct
{
    int quality;
    bool arith_code;
    J_DCT_METHOD dct_method;
    bool optimize;
    int smoothing;
} output_jpeg_options_t;
typedef struct
{
    image_type type;
    output_png_options_t png;
    output_jpeg_options_t jpeg;
} output_options_t;

typedef struct
{
    input_options_t input;
    randomizer_options_t randomizer;
    resizer_options_t resizer;
    output_options_t output;
} options_t;

typedef struct
{
    color_type color;
    uint32_t bit_depth; // bits per channel
    int channels;
    uint32_t width;
    uint32_t height;
    unsigned char *data; // pixel data

    uint32_t palette_len;
    unsigned char *palette; // for COLOR_PALETTE color mode
    uint32_t trns_len;
    unsigned char *trns; // transparency settings (png)
} image_t;

image_type str_to_type(const char *str);
int type_to_str(image_type type, char *str, size_t len);
int type_to_ext(image_type type, char *str, size_t len);
color_type png_to_color_type(enum spng_color_type color);
enum spng_color_type color_type_to_png(color_type color);
int color_type_to_channels(color_type color);
const char *color_type_to_str(color_type color);
color_type str_to_color_type(const char *str);

void im_shallow_copy(image_t *src, image_t *dst);

void debug_options(options_t options);
void debug_image(image_t *im);

options_t default_options();

int sanitize(unsigned char *data, size_t size, image_type type, const char *path, options_t options, char *res_path, size_t res_path_len);

int is_png(unsigned char *buffer, size_t len);
int is_jpeg(unsigned char *buffer, size_t len);

int png_decode(unsigned char *buffer, size_t buffer_size, uint32_t max_width, uint32_t max_height, size_t max_size, image_t **out_image);
int jpeg_decode(unsigned char *buffer, size_t buffer_size, uint32_t max_width, uint32_t max_height, size_t max_size, image_t **out_image);

int png_encode(const char *path, image_t *image, output_png_options_t options);
int jpeg_encode(const char *path, image_t *image, output_jpeg_options_t options);

int randomize_channels(image_t *image);
int randomize_channels_keep_trns(image_t *image);
int randomize_palette(image_t *image);

int resize(image_t *src, image_t **dst_ptr, uint32_t width, uint32_t height, resizer_type type);
int bilinear_interp(image_t *src, image_t **dst_ptr, uint32_t width, uint32_t height);

#define SUCCESS 0
#define ERROR_OUT_OF_MEMORY 1
#define ERROR_OPENING_FILE 2
#define ERROR_INTERNAL 3
#define ERROR_SET_SOURCE 4
#define ERROR_BAD_HEADER 5
#define ERROR_IMAGE_SIZE 6
#define ERROR_DECODE 7
#define ERROR_SET_DEST 8
#define ERROR_ENCODE 9
#define ERROR_NOT_SUPPORTED 10
#define ERROR_UNKNOWN_IMAGE_TYPE 11
#define ERROR_MISSING_PALETTE 12
#define ERROR_NOT_ALLOWED_TYPE 13