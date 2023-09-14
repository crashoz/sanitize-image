#include <time.h>
#include <stdlib.h>

#include <check.h>
#include <sanitize-image.h>

#include "tests-utils.h"

unsigned char *load_file(const char *path, size_t *len)
{
    unsigned char *buffer = malloc(BUFFER_SIZE);
    FILE *f;
    f = fopen(path, "rb");

    if (f == NULL)
    {
        perror("open file");
        exit(1);
    }

    int n = fread(buffer, sizeof(unsigned char), BUFFER_SIZE, f);
    buffer[n] = '\0';

    fclose(f);

    *len = n;

    return buffer;
}

int test_snapshot(const char *src, const char *snp, options_t options)
{
    size_t src_len;
    unsigned char *src_buffer = load_file(src, &src_len);
    unsigned char res_path[MAX_PATH];

    sanitize(src_buffer, src_len, TYPE_UNKNOWN, "./res", options, res_path, MAX_PATH);

    size_t dst_len;
    unsigned char *dst_buffer = load_file(res_path, &dst_len);

    size_t snp_len;
    unsigned char *snp_buffer = load_file(snp, &snp_len);

    ck_assert_mem_eq(dst_buffer, snp_buffer, snp_len);

    free(src_buffer);
    free(dst_buffer);
    free(snp_buffer);
}