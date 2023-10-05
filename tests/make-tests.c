#include <sys/stat.h>
#include <sys/types.h>

#include <sanitize-image.h>
#include <converters.h>

#include "tests-utils.h"

int make_convert_tests()
{
    const char *type_list[] = {"gray", "graya", "rgb", "rgba", "palette"};

    int res;
    size_t len;
    unsigned char *buffer;
    unsigned char path[MAX_PATH];
    unsigned char res_path[MAX_PATH];
    options_t options = default_options();

    for (int i = 0; i < 5; i++)
    {

        snprintf(path, MAX_PATH, "../../tests/snapshots/convert/%s", type_list[i]);
        res = mkdir(path, 0777);

        snprintf(path, MAX_PATH, "../../tests/base/%s.png", type_list[i]);
        snprintf(res_path, MAX_PATH, "../../tests/snapshots/convert/%s/src.png", type_list[i]);

        copy_file(path, res_path);

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
    }
}

int main(void)
{
    srand(1);
    init_convert_map();
    make_convert_tests();
}