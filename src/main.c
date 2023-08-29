#include <sanitize-image.h>
#include <spng.h>

#define BUFFER_SIZE 1024 * 1024 * 3

int main(int argc, char **argv)
{
    unsigned char *buffer = malloc(BUFFER_SIZE);
    FILE *f;
    f = fopen("../../lenna.jpg", "rb");

    int n = fread(buffer, sizeof(unsigned char), BUFFER_SIZE, f);
    buffer[n] = '\0';

    fclose(f);

    sanitize(buffer, n, "../../new.jpg");

    free(buffer);

    return 0;
}