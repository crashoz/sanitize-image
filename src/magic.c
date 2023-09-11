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

    return buffer[0] == 0x89 && buffer[1] == 0x50 && buffer[2] == 0x4E && buffer[3] == 0x47 &&
           buffer[4] == 0x0D && buffer[5] == 0x0A && buffer[6] == 0x1A && buffer[7] == 0x0A;
}