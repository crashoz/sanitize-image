#include <sys/stat.h>
#include <sys/types.h>

#include <sanitize-image.h>
#include <converters.h>

#include "tests-utils.h"

int make_convert_tests()
{
    int res;
    size_t len;
    unsigned char *buffer;
    unsigned char path[MAX_PATH];
    unsigned char res_path[MAX_PATH];
    options_t options = default_options();

    options.randomizer.type = RANDOMIZER_NONE;

    for (int i = 0; i < color_list_len; i++)
    {

        snprintf(path, MAX_PATH, "../../tests/snapshots/convert/%s", color_list[i]);
        res = mkdir(path, 0777);

        snprintf(path, MAX_PATH, "../../tests/base/%s.png", color_list[i]);

        buffer = load_file(path, &len);

        for (int j = 0; j < color_list_len; j++)
        {
            if (i == j)
            {
                continue;
            }

            options.output.png.color_type = str_to_color_type(color_list[j]);

            snprintf(path, MAX_PATH, "../../tests/snapshots/convert/%s/%s", color_list[i], color_list[j]);
            sanitize(buffer, len, TYPE_UNKNOWN, path, options, res_path, MAX_PATH);
        }

        free(buffer);
    }
}

int make_randomize_tests()
{
    int res;
    size_t len;
    unsigned char *buffer;
    unsigned char path[MAX_PATH];
    unsigned char res_path[MAX_PATH];
    options_t options = default_options();

    options.randomizer.type = RANDOMIZER_AUTO;

    for (int i = 0; i < color_list_len; i++)
    {
        srand(1);
        snprintf(path, MAX_PATH, "../../tests/base/%s.png", color_list[i]);

        buffer = load_file(path, &len);

        snprintf(path, MAX_PATH, "../../tests/snapshots/randomize/%s", color_list[i]);
        sanitize(buffer, len, TYPE_UNKNOWN, path, options, res_path, MAX_PATH);

        free(buffer);
    }
}

int make_resize_tests()
{
    int res;
    size_t len;
    unsigned char *buffer;
    unsigned char path[MAX_PATH];
    unsigned char res_path[MAX_PATH];
    options_t options = default_options();

    options.randomizer.type = RANDOMIZER_NONE;

    for (int i = 0; i < color_list_len; i++)
    {
        snprintf(path, MAX_PATH, "../../tests/snapshots/resize/%s", color_list[i]);
        res = mkdir(path, 0777);

        snprintf(path, MAX_PATH, "../../tests/base/%s.png", color_list[i]);

        buffer = load_file(path, &len);

        snprintf(path, MAX_PATH, "../../tests/snapshots/resize/%s/smaller", color_list[i]);
        options.resizer.type = RESIZER_AUTO;
        options.resizer.width = 20;
        options.resizer.height = 20;
        sanitize(buffer, len, TYPE_UNKNOWN, path, options, res_path, MAX_PATH);

        snprintf(path, MAX_PATH, "../../tests/snapshots/resize/%s/bigger", color_list[i]);
        options.resizer.type = RESIZER_AUTO;
        options.resizer.width = 40;
        options.resizer.height = 40;
        sanitize(buffer, len, TYPE_UNKNOWN, path, options, res_path, MAX_PATH);

        free(buffer);
    }
}

int make_types_tests()
{
    int res;
    size_t len;
    unsigned char *buffer;
    unsigned char path[MAX_PATH];
    unsigned char res_path[MAX_PATH];

    image_type type;
    unsigned char ext[8];

    options_t options = default_options();

    options.randomizer.type = RANDOMIZER_NONE;

    for (int i = 0; i < type_list_len; i++)
    {
        snprintf(path, MAX_PATH, "../../tests/snapshots/types/%s", type_list[i]);
        res = mkdir(path, 0777);

        type_to_ext(str_to_type(type_list[i]), ext, 8);

        snprintf(path, MAX_PATH, "../../tests/base/rgb%s", ext);
        buffer = load_file(path, &len);

        for (int j = 0; j < type_list_len; j++)
        {
            options.output.type = str_to_type(type_list[j]);

            snprintf(path, MAX_PATH, "../../tests/snapshots/types/%s/rgb", type_list[i]);
            sanitize(buffer, len, TYPE_UNKNOWN, path, options, res_path, MAX_PATH);
        }

        free(buffer);
    }
}

int main(void)
{
    srand(1);
    init_convert_map();

    make_convert_tests();
    make_randomize_tests();
    make_resize_tests();
    make_types_tests();
}