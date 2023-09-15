#include <time.h>
#include <stdlib.h>

#include <sanitize-image.h>
#include <spng.h>

#define BUFFER_SIZE 1024 * 1024 * 3

int main(int argc, char **argv)
{
    srand(1);

    unsigned char *buffer = malloc(BUFFER_SIZE);
    FILE *f;
    f = fopen("../../trns.png", "rb");

    int n = fread(buffer, sizeof(unsigned char), BUFFER_SIZE, f);
    buffer[n] = '\0';

    fclose(f);

    printf("size: %d\n", n);

    options_t options = default_options();
    options.randomizer.type = RANDOMIZER_NONE;
    options.resizer.type = RESIZER_BILINEAR;
    options.resizer.width = 32;
    options.resizer.height = 32;
    options.output.type = TYPE_PNG;
    char res_path[4096];
    sanitize(buffer, n, TYPE_PNG, "../../new", options, res_path, 4096);

    free(buffer);

    return 0;
}