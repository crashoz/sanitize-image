#include <sanitize-image.h>
#include <inttypes.h>

void randomize_channels(image_t *image)
{
    for (uint64_t i = 0; i < image->width * image->height * image->channels; i++)
    {
        int r = rand() % 3;
        switch (r)
        {
        case 0:
            if (image->data[i] > 0)
            {
                image->data[i]--;
            }
        case 1:
            if (image->data[i] < 255)
            {
                image->data[i]++;
            }
        }
    }
}

// TODO palette randomizer