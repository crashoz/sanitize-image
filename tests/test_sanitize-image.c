#include <stdlib.h>
#include <check.h>
#include <sanitize-image.h>

START_TEST(test_hello)
{
    ck_assert_int_eq(5, 5);
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
    tcase_add_test(tc_core, test_hello);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = sanitizeimage_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}