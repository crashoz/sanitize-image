#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <sanitize-image.h>
#include "quantizers.h"

#define clamp(v) (v) > 255 ? 255 : (v) < 0 ? 0 \
                                           : (v)

void debug_node(octree_node_t *node)
{
    printf("node: %p\n", node);
    printf("r, g, b: %d, %d, %d\n", node->r, node->g, node->b);
    printf("count: %d\n", node->count);
    printf("palette: %d\n", node->palette);
    int children_count = 0;
    for (int i = 0; i < 8; i++)
    {
        if (node->children[i] != NULL)
        {
            children_count++;
        }
    }
    printf("children: %d\n\n", children_count);
}

octree_node_t *octree_create_node()
{
    octree_node_t *node = calloc(1, sizeof(octree_node_t));
    if (node == NULL)
    {
        return NULL;
    }

    node->palette = 255;
    return node;
}

void octree_destroy(octree_node_t *node)
{
    if (node == NULL)
    {
        return;
    }

    for (int i = 0; i < 8; i++)
    {
        octree_destroy(node->children[i]);
    }

    free(node);
}

int octree_insert_color(octree_node_t *node, uint8_t r, uint8_t g, uint8_t b)
{
    for (int i = 0; i < 8; i++)
    {

        int idx = (((r >> (7 - i)) & 0x01) << 2) | (((g >> (7 - i)) & 0x01) << 1) | (((b >> (7 - i)) & 0x01));
        if (node->children[idx] == NULL)
        {
            node->children[idx] = octree_create_node();
            if (node->children[idx] == NULL)
            {
                return SZIM_ERROR_OUT_OF_MEMORY;
            }
        }
        node->children[idx]->parent = node;
        node = node->children[idx];
    }

    node->r += r;
    node->g += g;
    node->b += b;
    node->count++;

    return SUCCESS;
}

int octree_to_heap_rec(octree_node_t *node, heap_t *heap)
{
    int ret;
    if (node == NULL)
    {
        return SUCCESS;
    }

    if (node->count > 0)
    {
        if (heap->length >= heap->capacity)
        {
            ret = heap_grow(heap);
            if (ret != 0)
            {
                return ret;
            }
        }
        heap->arr[heap->length].priority = node->count;
        heap->arr[heap->length].octree_node = node;
        heap->length++;
    }
    else
    {
        for (int i = 0; i < 8; i++)
        {
            ret = octree_to_heap_rec(node->children[i], heap);
            if (ret != 0)
            {
                return ret;
            }
        }
    }
    return SUCCESS;
}

heap_t *octree_to_heap(octree_node_t *node)
{
    heap_t *heap = heap_create(512);
    if (heap == NULL)
    {
        return NULL;
    }

    if (octree_to_heap_rec(node, heap) != 0)
    {
        return NULL;
    }

    for (int i = (heap->length - 1) / 2; i > 0; i--)
    {
        heap_down(heap, i);
    }

    return heap;
}

void octree_reduce(heap_t *heap, uint8_t n_colors)
{
    uint32_t total_colors = heap->length;
    while (total_colors > n_colors)
    {
        heap_node_t *min_node = heap_peek(heap);
        octree_node_t *current = min_node->octree_node;
        octree_node_t *parent = current->parent;

        uint32_t prev_count = parent->count;

        parent->r += current->r;
        parent->g += current->g;
        parent->b += current->b;
        parent->count += current->count;

        int children_count = 0;
        for (int i = 0; i < 8; i++)
        {
            if (parent->children[i] == current)
            {
                parent->children[i] = NULL;
            }

            if (parent->children[i] != NULL)
            {
                children_count++;
            }
        }

        if (children_count == 0)
        {
            heap_node_t new = {parent->count, parent};
            heap_insert_extract(heap, new);
        }
        else
        {
            heap_extract(heap);
        }

        if (prev_count > 0)
        {
            total_colors--;
        }

        octree_destroy(current);
    }
}

int closest(unsigned char *palette, int palette_len, uint8_t r, uint8_t g, uint8_t b)
{
    int best = 3 * 255 * 255;
    int best_idx = -1;

    for (int i = 0; i < palette_len; i++)
    {
        int d = (r - palette[i * 3]) * (r - palette[i * 3]) + (g - palette[i * 3 + 1]) * (g - palette[i * 3 + 1]) + (b - palette[i * 3 + 2]) * (b - palette[i * 3 + 2]);
        if (d < best)
        {
            best = d;
            best_idx = i;
        }
    }

    return best_idx;
}

// Sierra Lite dithering
int dither(szim_image_t *image, unsigned char *palette, int palette_len, unsigned char *indexed_data)
{
    int32_t *dither = calloc(image->width * 2 * 3, sizeof(int32_t));
    if (dither == NULL)
    {
        return SZIM_ERROR_OUT_OF_MEMORY;
    }

    for (uint32_t k = 0; k < image->width * image->height; k++)
    {
        int32_t r_sum = image->data[k * 3] + dither[(k % image->width) * 3];
        int32_t g_sum = image->data[k * 3 + 1] + dither[(k % image->width) * 3 + 1];
        int32_t b_sum = image->data[k * 3 + 2] + dither[(k % image->width) * 3 + 2];

        uint8_t r = clamp(r_sum);
        uint8_t g = clamp(g_sum);
        uint8_t b = clamp(b_sum);

        int pidx = closest(palette, palette_len, r, g, b);
        indexed_data[k] = pidx;

        int quant_err_r = r - palette[pidx * 3];
        int quant_err_g = g - palette[pidx * 3 + 1];
        int quant_err_b = b - palette[pidx * 3 + 2];

        if (k % image->width < image->width - 1)
        {
            dither[(k % image->width + 1) * 3] += quant_err_r * 2 / 4;
            dither[(k % image->width + 1) * 3 + 1] += quant_err_g * 2 / 4;
            dither[(k % image->width + 1) * 3 + 2] += quant_err_b * 2 / 4;
        }
        else
        {
            memcpy(dither, dither + image->width * 3, image->width * 3 * sizeof(int32_t));
            memset(dither + image->width * 3, 0, image->width * 3 * sizeof(int32_t));
        }

        if (k % image->width > 0 && k / image->width < image->height - 1)
        {
            dither[(k % image->width - 1 + image->width) * 3] += quant_err_r * 1 / 4;
            dither[(k % image->width - 1 + image->width) * 3 + 1] += quant_err_g * 1 / 4;
            dither[(k % image->width - 1 + image->width) * 3 + 2] += quant_err_b * 1 / 4;
        }

        if (k / image->width < image->height - 1)
        {
            dither[(k % image->width + image->width) * 3] += quant_err_r * 1 / 4;
            dither[(k % image->width + image->width) * 3 + 1] += quant_err_g * 1 / 4;
            dither[(k % image->width + image->width) * 3 + 2] += quant_err_b * 1 / 4;
        }
    }

    free(dither);

    return SUCCESS;
}

void octree_palette(szim_image_t *image, octree_node_t *octree, int n_colors, unsigned char *indexed_data, unsigned char *palette, int *plte_len_res)
{
    octree_node_t *node;
    int plte_len = 0;

    for (uint32_t k = 0; k < image->width * image->height; k++)
    {
        uint8_t r = image->data[k * 3];
        uint8_t g = image->data[k * 3 + 1];
        uint8_t b = image->data[k * 3 + 2];

        node = octree;
        for (int i = 0; i < 9; i++)
        {
            int idx = (((r >> (7 - i)) & 0x01) << 2) | (((g >> (7 - i)) & 0x01) << 1) | (((b >> (7 - i)) & 0x01));

            if (node->children[idx] == NULL)
            {
                if (node->palette == 255)
                {
                    palette[plte_len * 3] = node->r / node->count;
                    palette[plte_len * 3 + 1] = node->g / node->count;
                    palette[plte_len * 3 + 2] = node->b / node->count;

                    node->palette = plte_len;

                    plte_len++;
                }

                indexed_data[k] = node->palette;
                break;
            }
            node = node->children[idx];
        }
    }

    *plte_len_res = plte_len;
}

heap_t *heap_create(size_t capacity)
{
    heap_t *heap = malloc(sizeof(heap_t));
    if (heap == NULL)
    {
        return NULL;
    }
    heap->arr = malloc(capacity * sizeof(heap_node_t));
    if (heap->arr == NULL)
    {
        return NULL;
    }
    heap->length = 1;
    heap->capacity = capacity;
    return heap;
}

void heap_destroy(heap_t *heap)
{
    free(heap->arr);
    free(heap);
}

int heap_grow(heap_t *heap)
{
    heap->capacity *= 2;
    heap->arr = realloc(heap->arr, heap->capacity * sizeof(heap_node_t));
    if (heap->arr == NULL)
    {
        return SZIM_ERROR_OUT_OF_MEMORY;
    }

    return SUCCESS;
}

void heap_swap(heap_t *heap, uint32_t i, uint32_t j)
{
    heap_node_t tmp = heap->arr[i];
    heap->arr[i] = heap->arr[j];
    heap->arr[j] = tmp;
}

int heap_insert(heap_t *heap, heap_node_t elt)
{
    int ret;
    if (heap->length >= heap->capacity)
    {
        ret = heap_grow(heap);
        if (ret != 0)
        {
            return ret;
        }
    }

    heap->arr[heap->length] = elt;
    uint32_t i = heap->length++;

    while (i > 1 && heap->arr[i / 2].priority > heap->arr[i].priority)
    {
        heap_swap(heap, i / 2, i);
        i /= 2;
    }

    return SUCCESS;
}

void heap_down(heap_t *heap, uint32_t i)
{
    uint32_t smallest = i;
    while (true)
    {
        if (2 * i < heap->length && heap->arr[2 * i].priority < heap->arr[i].priority)
        {
            smallest = 2 * i;
        }

        if (2 * i + 1 < heap->length && heap->arr[2 * i + 1].priority < heap->arr[smallest].priority)
        {
            smallest = 2 * i + 1;
        }

        if (smallest == i)
        {
            break;
        }
        heap_swap(heap, i, smallest);
        i = smallest;
    }
}

heap_node_t *heap_peek(heap_t *heap)
{
    return &heap->arr[1];
}

heap_node_t heap_extract(heap_t *heap)
{
    heap_node_t min = heap->arr[1];
    heap->arr[1] = heap->arr[heap->length - 1];
    heap->length--;

    heap_down(heap, 1);
    return min;
}

heap_node_t heap_insert_extract(heap_t *heap, heap_node_t elt)
{
    if (elt.priority < heap->arr[1].priority)
    {
        return elt;
    }

    heap_node_t min = heap->arr[1];
    heap->arr[1] = elt;

    heap_down(heap, 1);
    return min;
}

void quantize_rgb(szim_image_t *image)
{
    int n_colors = 255;
    octree_node_t *octree = octree_create_node();

    for (uint32_t i = 0; i < image->width * image->height; i++)
    {
        octree_insert_color(octree, image->data[i * 3], image->data[i * 3 + 1], image->data[i * 3 + 2]);
    }

    heap_t *heap = octree_to_heap(octree);
    printf("heap: %d\n", heap->length);

    octree_reduce(heap, n_colors);
    printf("heap: %d\n", heap->length);

    unsigned char *indexed_data = malloc(image->width * image->height);
    unsigned char *palette = malloc(n_colors * 3);
    int plte_len;
    octree_palette(image, octree, n_colors, indexed_data, palette, &plte_len);
    dither(image, palette, plte_len, indexed_data);

    free(image->data);
    image->data = indexed_data;
    image->palette = palette;
    image->palette_len = plte_len;
    image->color = SZIM_COLOR_PALETTE;
    image->channels = 1;
    image->bit_depth = 8;

    heap_destroy(heap);
    octree_destroy(octree);
}