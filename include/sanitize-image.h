#pragma once

#include <inttypes.h>
#include <spng.h>

typedef enum
{
    TYPE_UNKNOWN,
    TYPE_INPUT,
    TYPE_PNG,
    TYPE_JPEG
} image_type;

typedef struct
{
    image_type allowed_types[3];
    uint32_t max_width;
    uint32_t max_height;
    size_t max_size;
} input_options_t;

typedef enum
{
    RANDOMIZER_NONE,
    RANDOMIZER_AUTO,
    RANDOMIZER_DEFAULT
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
    enum spng_color_type color_type;
    int bit_depth;
} output_png_options_t;

typedef struct
{
    int quality;
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
    uint32_t width;
    uint32_t height;
    unsigned char *data;
} image_t;

options_t default_options();
int sanitize(unsigned char *data, size_t size, image_type type, const char *path, options_t options);

int is_png(unsigned char *buffer, size_t len);
int is_jpeg(unsigned char *buffer, size_t len);

int png_decode(unsigned char *buffer, size_t buffer_size, uint32_t max_width, uint32_t max_height, size_t max_size, image_t **out_image);
int jpeg_decode(unsigned char *buffer, size_t buffer_size, uint32_t max_width, uint32_t max_height, size_t max_size, image_t **out_image);

int png_encode(const char *path, image_t *image, enum spng_color_type color_type, int bit_depth);
int jpeg_encode(const char *path, image_t *image, int quality);

void randomize_rgb(image_t *image);

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
#define UNKNOWN_IMAGE_TYPE 11