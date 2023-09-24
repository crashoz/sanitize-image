#pragma once

#include <stdint.h>

int gray_to_graya(image_t *src, image_t **dst);
int gray_to_rgb(image_t *src, image_t **dst);
int gray_to_rgba(image_t *src, image_t **dst);
int gray_to_palette(image_t *src, image_t **dst);

int graya_to_gray(image_t *src, image_t **dst);
int graya_to_rgb(image_t *src, image_t **dst);
int graya_to_rgba(image_t *src, image_t **dst);
int graya_to_palette(image_t *src, image_t **dst);

int rgb_to_gray(image_t *src, image_t **dst);
int rgb_to_graya(image_t *src, image_t **dst);
int rgb_to_rgba(image_t *src, image_t **dst);
int rgb_to_palette(image_t *src, image_t **dst);

int rgba_to_gray(image_t *src, image_t **dst);
int rgba_to_graya(image_t *src, image_t **dst);
int rgba_to_rgb(image_t *src, image_t **dst);
int rgba_to_palette(image_t *src, image_t **dst);

int palette_to_gray(image_t *src, image_t **dst);
int palette_to_graya(image_t *src, image_t **dst);
int palette_to_rgb(image_t *src, image_t **dst);
int palette_to_rgba(image_t *src, image_t **dst);
