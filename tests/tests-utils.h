#pragma once

#include <stdlib.h>
#include <sanitize-image.h>

#define BUFFER_SIZE 2048 * 2048 * 4
#define MAX_PATH 4096

extern const char *color_list[];
extern const char *type_list[];

extern const int color_list_len;
extern const int type_list_len;

int copy_file(const char *in_path, const char *out_path);
unsigned char *load_file(const char *path, size_t *len);
int test_snapshot(const char *src, const char *snp, options_t options);