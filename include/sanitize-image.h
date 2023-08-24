#pragma once

#include <inttypes.h>
#include <spng.h>

typedef struct
{
    uint32_t width;
    uint32_t height;
    unsigned char *data;
} image_t;

void hello_sanitizer();

int png_decode(char *path, uint32_t max_width, uint32_t max_height, size_t max_size, image_t **out_image);
int jpeg_decode(char *path, uint32_t max_width, uint32_t max_height, size_t max_size, image_t **out_image);

int png_encode(char *path, image_t *image, enum spng_color_type color_type, int bit_depth);
int jpeg_encode(char *path, image_t *image, int quality);

void randomize_rgb(image_t *image);

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