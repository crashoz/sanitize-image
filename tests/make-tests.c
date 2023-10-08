#include <sys/stat.h>
#include <sys/types.h>

#include <sanitize-image.h>
#include <converters.h>

#include "tests-utils.h"

const char *type_list[] = {"gray", "graya", "rgb", "rgba", "palette"};

int make_convert_tests()
{
    int res;
    size_t len;
    unsigned char *buffer;
    unsigned char path[MAX_PATH];
    unsigned char res_path[MAX_PATH];
    options_t options = default_options();

    options.randomizer.type = RANDOMIZER_NONE;

    for (int i = 0; i < 5; i++)
    {

        snprintf(path, MAX_PATH, "../../tests/snapshots/convert/%s", type_list[i]);
        res = mkdir(path, 0777);

        snprintf(path, MAX_PATH, "../../tests/base/%s.png", type_list[i]);

        buffer = load_file(path, &len);

        for (int j = 0; j < 5; j++)
        {
            if (i == j)
            {
                continue;
            }

            options.output.png.color_type = str_to_color_type(type_list[j]);

            snprintf(path, MAX_PATH, "../../tests/snapshots/convert/%s/%s", type_list[i], type_list[j]);
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

    for (int i = 0; i < 5; i++)
    {
        snprintf(path, MAX_PATH, "../../tests/base/%s.png", type_list[i]);

        buffer = load_file(path, &len);

        snprintf(path, MAX_PATH, "../../tests/snapshots/randomize/%s", type_list[i]);
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

    for (int i = 0; i < 5; i++)
    {
        snprintf(path, MAX_PATH, "../../tests/snapshots/resize/%s", type_list[i]);
        res = mkdir(path, 0777);

        snprintf(path, MAX_PATH, "../../tests/base/%s.png", type_list[i]);

        buffer = load_file(path, &len);

        snprintf(path, MAX_PATH, "../../tests/snapshots/resize/%s/smaller", type_list[i]);
        options.resizer.type = RESIZER_AUTO;
        options.resizer.width = 20;
        options.resizer.height = 20;
        sanitize(buffer, len, TYPE_UNKNOWN, path, options, res_path, MAX_PATH);

        snprintf(path, MAX_PATH, "../../tests/snapshots/resize/%s/bigger", type_list[i]);
        options.resizer.type = RESIZER_AUTO;
        options.resizer.width = 40;
        options.resizer.height = 40;
        sanitize(buffer, len, TYPE_UNKNOWN, path, options, res_path, MAX_PATH);

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
}