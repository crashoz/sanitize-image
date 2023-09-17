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
    f = fopen("../../comp.png", "rb");

    int n = fread(buffer, sizeof(unsigned char), BUFFER_SIZE, f);
    buffer[n] = '\0';

    fclose(f);

    printf("size: %d\n", n);

    options_t options = default_options();
    options.output.type = TYPE_JPEG;
    char res_path[4096];
    sanitize(buffer, n, TYPE_UNKNOWN, "../../new", options, res_path, 4096);

    free(buffer);

    return 0;
}