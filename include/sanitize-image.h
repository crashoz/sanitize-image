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