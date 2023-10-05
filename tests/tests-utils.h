#pragma once

#include <stdlib.h>

#define BUFFER_SIZE 512 * 512 * 4
#define MAX_PATH 4096

int copy_file(const char *in_path, const char *out_path);
unsigned char *load_file(const char *path, size_t *len);
int test_snapshot(const char *src, const char *snp, options_t options);