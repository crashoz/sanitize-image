#include <sanitize-image.h>
#include <inttypes.h>

int randomize_channels(image_t *image)
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

    return SUCCESS;
}

int randomize_palette(image_t *image)
{
    unsigned char mapping[image->palette_len];

    // Fisher-Yates "inside out" shuffle
    for (int i = 0; i < image->palette_len; i++)
    {
        int j = rand() % (i + 1);
        if (j != i)
        {
            mapping[i] = mapping[j];
        }
        mapping[j] = i;
    }

    unsigned char *mapped_palette = malloc(image->palette_len * 3);
    if (mapped_palette == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    for (int i = 0; i < image->palette_len; i++)
    {
        int j = mapping[i];

        mapped_palette[j * 3] = image->palette[i * 3];
        mapped_palette[j * 3 + 1] = image->palette[i * 3 + 1];
        mapped_palette[j * 3 + 2] = image->palette[i * 3 + 2];
    }

    free(image->palette);
    image->palette = mapped_palette;

    for (uint64_t i = 0; i < image->width * image->height; i++)
    {
        image->data[i] = mapping[image->data[i]];
    }

    return SUCCESS;
}