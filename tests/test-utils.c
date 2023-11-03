#include <time.h>
#include <stdlib.h>

#include <check.h>
#include <sanitize-image.h>

#include "tests-utils.h"

const char *color_list[] = {"gray", "graya", "rgb", "rgba", "palette"};
const char *type_list[] = {"png", "jpeg"};

const int color_list_len = sizeof(color_list) / sizeof(color_list[0]);
const int type_list_len = sizeof(type_list) / sizeof(type_list[0]);

// https://stackoverflow.com/a/68695871/22471816
int copy_file(const char *in_path, const char *out_path)
{
    size_t n;
    FILE *in = NULL, *out = NULL;
    char *buf = calloc(BUFFER_SIZE, 1);
    if ((in = fopen(in_path, "rb")) && (out = fopen(out_path, "wb")))
        while ((n = fread(buf, 1, BUFFER_SIZE, in)) && fwrite(buf, 1, n, out))
            ;
    free(buf);
    if (in)
        fclose(in);
    if (out)
        fclose(out);
    return EXIT_SUCCESS;
}

unsigned char *load_file(const char *path, size_t *len)
{
    unsigned char *buffer = malloc(BUFFER_SIZE);
    FILE *f;
    f = fopen(path, "rb");

    if (f == NULL)
    {
        ck_abort_msg("cannot open file: %s", path);
    }

    int n = fread(buffer, sizeof(unsigned char), BUFFER_SIZE, f);
    buffer[n] = '\0';

    fclose(f);

    *len = n;

    return buffer;
}

int test_snapshot(const char *src, const char *snp, szim_options_t options)
{
    size_t src_len;
    unsigned char *src_buffer = load_file(src, &src_len);
    unsigned char res_path[MAX_PATH];

    szim_sanitize(src_buffer, src_len, SZIM_TYPE_UNKNOWN, "./res", options, res_path, MAX_PATH);

    size_t dst_len;
    unsigned char *dst_buffer = load_file(res_path, &dst_len);

    size_t snp_len;
    unsigned char *snp_buffer = load_file(snp, &snp_len);

    int ret = memcmp(dst_buffer, snp_buffer, snp_len);

    free(src_buffer);
    free(dst_buffer);
    free(snp_buffer);

    return ret == 0;
}