#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <spng.h>
#include <jpeglib.h>

#define MAX_ALLOWED_TYPES 8

typedef enum
{
    SZIM_TYPE_UNKNOWN,
    SZIM_TYPE_INPUT,
    SZIM_TYPE_PNG,
    SZIM_TYPE_JPEG
} szim_image_type;

typedef enum
{
    SZIM_COLOR_UNKNOWN,
    SZIM_COLOR_INPUT,
    SZIM_COLOR_GRAY,
    SZIM_COLOR_GRAYA,
    SZIM_COLOR_RGB,
    SZIM_COLOR_RGBA,
    SZIM_COLOR_PALETTE
} szim_color_type;

typedef struct
{
    szim_image_type allowed_types[MAX_ALLOWED_TYPES];
    uint32_t max_width;
    uint32_t max_height;
    size_t max_size;
} szim_input_options_t;

typedef enum
{
    SZIM_RANDOMIZER_NONE,
    SZIM_RANDOMIZER_AUTO
} szim_randomizer_type;

typedef struct
{
    szim_randomizer_type type;
} szim_randomizer_options_t;
typedef enum
{
    SZIM_RESIZER_NONE,
    SZIM_RESIZER_NN,
    SZIM_RESIZER_BILINEAR,
    SZIM_RESIZER_AUTO
} szim_resizer_type;
typedef struct
{
    szim_resizer_type type;
    uint32_t width;
    uint32_t height;
} szim_resizer_options_t;

typedef struct
{
    szim_color_type color_type;
    int compression_level;
    enum spng_filter_choice filter;
    bool interlace;
} szim_output_png_options_t;

typedef struct
{
    int quality;
    bool arith_code;
    J_DCT_METHOD dct_method;
    bool optimize;
    int smoothing;
    bool progressive;
} szim_output_jpeg_options_t;

typedef struct
{
    szim_image_type type;
    szim_output_png_options_t png;
    szim_output_jpeg_options_t jpeg;
} szim_output_options_t;

typedef struct
{
    szim_input_options_t input;
    szim_randomizer_options_t randomizer;
    szim_resizer_options_t resizer;
    szim_output_options_t output;
} szim_options_t;

typedef struct
{
    szim_color_type color;
    uint32_t bit_depth; // bits per channel
    int channels;
    uint32_t width;
    uint32_t height;
    unsigned char *data; // pixel data

    uint32_t palette_len;
    unsigned char *palette; // for COLOR_PALETTE color mode
    uint32_t trns_len;
    unsigned char *trns; // transparency settings (png)
} szim_image_t;

void init_convert_map();

void destroy_image(szim_image_t *im);

szim_image_type str_to_type(const char *str);
int type_to_str(szim_image_type type, char *str, size_t len);
int type_to_ext(szim_image_type type, char *str, size_t len);
szim_color_type png_to_color_type(enum spng_color_type color);
enum spng_color_type color_type_to_png(szim_color_type color);
int color_type_to_channels(szim_color_type color);
const char *color_type_to_str(szim_color_type color);
szim_color_type str_to_color_type(const char *str);

void im_shallow_copy(szim_image_t *src, szim_image_t *dst);
void im_deep_copy(szim_image_t *src, szim_image_t *dst);

void debug_options(szim_options_t options);
void debug_image(szim_image_t *im);

szim_options_t szim_default_options();

int szim_sanitize(unsigned char *data, size_t size, szim_image_type type, const char *path, szim_options_t options, char *res_path, size_t res_path_len);

int is_png(unsigned char *buffer, size_t len);
int is_jpeg(unsigned char *buffer, size_t len);

int png_decode(unsigned char *buffer, size_t buffer_size, uint32_t max_width, uint32_t max_height, size_t max_size, szim_image_t **out_image);
int jpeg_decode(unsigned char *buffer, size_t buffer_size, uint32_t max_width, uint32_t max_height, size_t max_size, szim_image_t **out_image);

int png_encode(const char *path, szim_image_t *image, szim_output_png_options_t options);
int jpeg_encode(const char *path, szim_image_t *image, szim_output_jpeg_options_t options);

int randomize_channels(szim_image_t *image);
int randomize_channels_keep_trns(szim_image_t *image);
int randomize_palette(szim_image_t *image);

int resize(szim_image_t *src, szim_image_t **dst_ptr, uint32_t width, uint32_t height, szim_resizer_type type);
int nn_interp(szim_image_t *src, szim_image_t **dst_ptr, uint32_t width, uint32_t height);
int bilinear_interp(szim_image_t *src, szim_image_t **dst_ptr, uint32_t width, uint32_t height);

#define SUCCESS 0
#define SZIM_ERROR_OUT_OF_MEMORY 1
#define SZIM_ERROR_OPENING_FILE 2
#define SZIM_ERROR_INTERNAL 3
#define SZIM_ERROR_SET_SOURCE 4
#define SZIM_ERROR_BAD_HEADER 5
#define SZIM_ERROR_IMAGE_SIZE 6
#define SZIM_ERROR_DECODE 7
#define SZIM_ERROR_SET_DEST 8
#define SZIM_ERROR_ENCODE 9
#define SZIM_ERROR_NOT_SUPPORTED 10
#define SZIM_ERROR_UNKNOWN_IMAGE_TYPE 11
#define SZIM_ERROR_MISSING_PALETTE 12
#define SZIM_ERROR_NOT_ALLOWED_TYPE 13