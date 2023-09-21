#include <sanitize-image.h>
#include "tests-utils.h"

int main(void)
{
    size_t len;
    unsigned char *buffer;
    unsigned char res_path[MAX_PATH];
    options_t options = default_options();

    buffer = load_file("../../tests/lenna.png", &len);
    sanitize(buffer, len, TYPE_UNKNOWN, "../../tests/new", options, res_path, MAX_PATH);
}