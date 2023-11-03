#pragma once

#include <stdint.h>
#include <sanitize-image.h>

typedef int (*convert_fn)(szim_image_t *, szim_image_t **);

extern convert_fn convert_map[5][5];

void init_convert_map();

int identity(szim_image_t *src, szim_image_t **dst);

int gray_to_graya(szim_image_t *src, szim_image_t **dst);
int gray_to_rgb(szim_image_t *src, szim_image_t **dst);
int gray_to_rgba(szim_image_t *src, szim_image_t **dst);
int gray_to_palette(szim_image_t *src, szim_image_t **dst);

int graya_to_gray(szim_image_t *src, szim_image_t **dst);
int graya_to_rgb(szim_image_t *src, szim_image_t **dst);
int graya_to_rgba(szim_image_t *src, szim_image_t **dst);
int graya_to_palette(szim_image_t *src, szim_image_t **dst);

int rgb_to_gray(szim_image_t *src, szim_image_t **dst);
int rgb_to_graya(szim_image_t *src, szim_image_t **dst);
int rgb_to_rgba(szim_image_t *src, szim_image_t **dst);
int rgb_to_palette(szim_image_t *src, szim_image_t **dst);

int rgba_to_gray(szim_image_t *src, szim_image_t **dst);
int rgba_to_graya(szim_image_t *src, szim_image_t **dst);
int rgba_to_rgb(szim_image_t *src, szim_image_t **dst);
int rgba_to_palette(szim_image_t *src, szim_image_t **dst);

int palette_to_gray(szim_image_t *src, szim_image_t **dst);
int palette_to_graya(szim_image_t *src, szim_image_t **dst);
int palette_to_rgb(szim_image_t *src, szim_image_t **dst);
int palette_to_rgba(szim_image_t *src, szim_image_t **dst);
