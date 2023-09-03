#include <sanitize-image.h>

/* Guess image type from the first bytes of the header */

int is_jpeg(unsigned char *buffer, size_t len)
{
    if (len < 4)
    {
        return 0;
    }

    return buffer[0] == 0xFF && buffer[1] == 0xD8 && buffer[2] == 0xFF &&
           (buffer[3] == 0xE0 || buffer[3] == 0xE1 || buffer[3] == 0xEE);
}

int is_png(unsigned char *buffer, size_t len)
{
    if (len < 8)
    {
        return 0;
    }

    return buffer[0] == 0x89 && buffer[0] == 0x50 && buffer[0] == 0x4E && buffer[0] == 0x47 &&
           buffer[0] == 0x0D && buffer[0] == 0x0A && buffer[0] == 0x1A && buffer[0] == 0x0A;
}