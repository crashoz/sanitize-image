#include <time.h>
#include <stdlib.h>

#include <sanitize-image.h>
#include <converters.h>
#include <spng.h>

#define BUFFER_SIZE 1024 * 1024 * 3

int main(int argc, char **argv)
{
    srand(1);

    init_convert_map();

    unsigned char *buffer = malloc(BUFFER_SIZE);
    FILE *f;
    f = fopen("../../lenna.png", "rb");
    // f = fopen("../../rgb.png", "rb");

    int n = fread(buffer, sizeof(unsigned char), BUFFER_SIZE, f);
    buffer[n] = '\0';

    fclose(f);

    printf("size: %d\n", n);

    options_t options = default_options();
    options.randomizer.type = RANDOMIZER_NONE;
    options.resizer.type = RESIZER_NN;
    options.resizer.width = 900;
    options.resizer.height = 900;
    options.output.type = TYPE_PNG;
    char res_path[4096];
    int ret = sanitize(buffer, n, TYPE_UNKNOWN, "../../new", options, res_path, 4096);

    if (ret != 0)
    {
        printf("error: %d\n", ret);
    }

    free(buffer);

    return 0;
}