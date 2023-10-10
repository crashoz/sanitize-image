#include <time.h>
#include <stdlib.h>

#include <check.h>
#include <sanitize-image.h>

#include "tests-utils.h"

// TODO trns

START_TEST(convert_colors)
{
    unsigned char input[MAX_PATH];
    unsigned char output[MAX_PATH];

    snprintf(input, MAX_PATH, "../../tests/base/%s.png", color_list[_i]);

    options_t options = default_options();
    options.randomizer.type = RANDOMIZER_NONE;
    options.output.type = TYPE_PNG;

    for (int j = 0; j < color_list_len; j++)
    {
        if (_i == j)
        {
            continue;
        }

        options.output.png.color_type = str_to_color_type(color_list[j]);

        snprintf(output, MAX_PATH, "../../tests/snapshots/convert/%s/%s.png", color_list[_i], color_list[j]);

        ck_assert_msg(test_snapshot(input, output, options), "convert %s -> %s", color_list[_i], color_list[j]);
    }
}
END_TEST

START_TEST(randomizer)
{
    unsigned char input[MAX_PATH];
    unsigned char output[MAX_PATH];

    snprintf(input, MAX_PATH, "../../tests/base/%s.png", color_list[_i]);

    options_t options = default_options();
    options.randomizer.type = RANDOMIZER_AUTO;

    snprintf(output, MAX_PATH, "../../tests/snapshots/randomize/%s.png", color_list[_i]);

    srand(1);
    ck_assert_msg(test_snapshot(input, output, options), "randomize %s", color_list[_i]);
}
END_TEST

START_TEST(resizer)
{
    unsigned char input[MAX_PATH];
    unsigned char output[MAX_PATH];

    snprintf(input, MAX_PATH, "../../tests/base/%s.png", color_list[_i]);

    options_t options = default_options();
    options.randomizer.type = RANDOMIZER_NONE;

    options.resizer.type = RESIZER_AUTO;
    options.resizer.width = 20;
    options.resizer.height = 20;

    snprintf(output, MAX_PATH, "../../tests/snapshots/resize/%s/smaller.png", color_list[_i]);

    ck_assert_msg(test_snapshot(input, output, options), "resizer 20x20 %s", color_list[_i]);

    options.resizer.type = RESIZER_AUTO;
    options.resizer.width = 40;
    options.resizer.height = 40;

    snprintf(output, MAX_PATH, "../../tests/snapshots/resize/%s/bigger.png", color_list[_i]);

    ck_assert_msg(test_snapshot(input, output, options), "resizer 40x40 %s", color_list[_i]);
}
END_TEST

START_TEST(convert_types)
{
    unsigned char input[MAX_PATH];
    unsigned char output[MAX_PATH];

    unsigned char ext[8];

    type_to_ext(str_to_type(type_list[_i]), ext, 8);

    snprintf(input, MAX_PATH, "../../tests/base/rgb%s", ext);

    options_t options = default_options();
    options.randomizer.type = RANDOMIZER_NONE;

    for (int j = 0; j < type_list_len; j++)
    {
        type_to_ext(str_to_type(type_list[j]), ext, 8);

        options.output.type = str_to_type(type_list[j]);

        snprintf(output, MAX_PATH, "../../tests/snapshots/types/%s/rgb%s", type_list[_i], ext);
        ck_assert_msg(test_snapshot(input, output, options), "convert %s -> %s", type_list[_i], type_list[j]);
    }
}
END_TEST

Suite *sanitizeimage_suite(void)
{
    Suite *s;
    TCase *tc_convert_colors, *tc_randomizer, *tc_resizer, *tc_convert_types;

    s = suite_create("Sanitize Image");

    tc_convert_colors = tcase_create("convert colors");
    tcase_add_loop_test(tc_convert_colors, convert_colors, 0, color_list_len);
    suite_add_tcase(s, tc_convert_colors);

    tc_randomizer = tcase_create("randomizer");
    tcase_add_loop_test(tc_randomizer, randomizer, 0, color_list_len);
    suite_add_tcase(s, tc_randomizer);

    tc_resizer = tcase_create("resizer");
    tcase_add_loop_test(tc_resizer, resizer, 0, color_list_len);
    suite_add_tcase(s, tc_resizer);

    tc_convert_types = tcase_create("convert image types");
    tcase_add_loop_test(tc_convert_types, convert_types, 0, type_list_len);
    suite_add_tcase(s, tc_convert_types);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    srand(1);
    init_convert_map();

    s = sanitizeimage_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}