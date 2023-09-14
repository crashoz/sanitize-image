#pragma once

#include <stdlib.h>

#define BUFFER_SIZE 512 * 512 * 4
#define MAX_PATH 4096

unsigned char *load_file(const char *path, size_t *len);
int test_snapshot(const char *src, const char *snp, options_t options);