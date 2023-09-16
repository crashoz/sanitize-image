#include <time.h>
#include <stdlib.h>

#include <check.h>
#include <sanitize-image.h>

#include "tests-utils.h"

START_TEST(png_rgb)
{
    options_t options = default_options();
    options.randomizer.type = RANDOMIZER_NONE;
    options.output.type = TYPE_INPUT;

    test_snapshot("../tests/snapshots/png/rgb.png", "../tests/snapshots/png/rgb.png", options);
}
END_TEST

START_TEST(png_rgba)
{
    options_t options = default_options();
    options.randomizer.type = RANDOMIZER_NONE;
    options.output.type = TYPE_INPUT;

    test_snapshot("../tests/snapshots/png/rgba.png", "../tests/snapshots/png/rgba.png", options);
}
END_TEST

START_TEST(png_grayscale)
{
    options_t options = default_options();
    options.randomizer.type = RANDOMIZER_NONE;
    options.output.type = TYPE_INPUT;

    test_snapshot("../tests/snapshots/png/grayscale.png", "../tests/snapshots/png/grayscale.png", options);
}
END_TEST

START_TEST(png_grayscale_alpha)
{
    options_t options = default_options();
    options.randomizer.type = RANDOMIZER_NONE;
    options.output.type = TYPE_INPUT;

    test_snapshot("../tests/snapshots/png/grayscale_alpha.png", "../tests/snapshots/png/grayscale_alpha.png", options);
}
END_TEST

START_TEST(png_palette)
{
    options_t options = default_options();
    options.randomizer.type = RANDOMIZER_NONE;
    options.output.type = TYPE_INPUT;

    test_snapshot("../tests/snapshots/png/palette.png", "../tests/snapshots/png/palette.png", options);
}
END_TEST

START_TEST(png_trns)
{
    options_t options = default_options();
    options.randomizer.type = RANDOMIZER_NONE;
    options.output.type = TYPE_INPUT;

    test_snapshot("../tests/snapshots/png/trns.png", "../tests/snapshots/png/trns.png", options);
}
END_TEST

START_TEST(jpeg_rgb)
{
    options_t options = default_options();
    options.randomizer.type = RANDOMIZER_NONE;
    options.output.type = TYPE_INPUT;

    test_snapshot("../tests/snapshots/jpeg/rgb.jpg", "../tests/snapshots/jpeg/rgb.jpg", options);
}
END_TEST

Suite *sanitizeimage_suite(void)
{
    Suite *s;
    TCase *tc_png_through;
    TCase *tc_jpeg_through;

    s = suite_create("Sanitize Image");

    tc_png_through = tcase_create("png pass through");

    tcase_add_test(tc_png_through, png_rgb);
    tcase_add_test(tc_png_through, png_rgba);
    tcase_add_test(tc_png_through, png_grayscale);
    tcase_add_test(tc_png_through, png_grayscale_alpha);
    tcase_add_test(tc_png_through, png_palette);
    tcase_add_test(tc_png_through, png_trns);

    suite_add_tcase(s, tc_png_through);

    tc_jpeg_through = tcase_create("jpeg pass through");

    tcase_add_test(tc_jpeg_through, jpeg_rgb);

    suite_add_tcase(s, tc_jpeg_through);

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