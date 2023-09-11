#include <time.h>
#include <stdlib.h>

#include <check.h>
#include <sanitize-image.h>

#define BUFFER_SIZE 512 * 512 * 4
#define MAX_PATH 4096

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

START_TEST(png_rgb)
{
    options_t options = default_options();
    options.randomizer.type = RANDOMIZER_NONE;
    options.output.type = TYPE_INPUT;

    size_t src_len;
    unsigned char *src_buffer = load_file("../../tests/snapshots/png/rgb.png", &src_len);
    unsigned char res_path[MAX_PATH];

    sanitize(src_buffer, src_len, TYPE_UNKNOWN, "./res", options, res_path, MAX_PATH);

    size_t dst_len;
    unsigned char *dst_buffer = load_file(res_path, &dst_len);

    size_t snp_len;
    unsigned char *snp_buffer = load_file("../../tests/snapshots/png/rgb.png", &snp_len);

    ck_assert_mem_eq(dst_buffer, snp_buffer, snp_len);

    free(src_buffer);
    free(dst_buffer);
    free(snp_buffer);
}
END_TEST

Suite *sanitizeimage_suite(void)
{
    Suite *s;
    TCase *tc_core;
    TCase *tc_limits;

    s = suite_create("Sanitize Image");

    /* Core test case */
    tc_core = tcase_create("Core");

    // tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, png_rgb);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    srand(1);

    s = sanitizeimage_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}