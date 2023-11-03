#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <sanitize-image.h>

struct octree_node
{
    uint32_t count;
    uint32_t r, g, b;
    uint8_t palette;
    struct octree_node *parent;
    struct octree_node *children[8];
};

typedef struct octree_node octree_node_t;

octree_node_t *octree_create_node();
void octree_destroy(octree_node_t *node);
int octree_insert_color(octree_node_t *node, uint8_t r, uint8_t g, uint8_t b);

typedef struct
{
    uint32_t priority;
    octree_node_t *octree_node;
} heap_node_t;

typedef struct
{
    heap_node_t *arr;
    uint32_t length;
    uint32_t capacity;
} heap_t;

heap_t *heap_create(size_t capacity);
void heap_destroy(heap_t *heap);
int heap_grow(heap_t *heap);
void heap_swap(heap_t *heap, uint32_t i, uint32_t j);
int heap_insert(heap_t *heap, heap_node_t elt);
void heap_down(heap_t *heap, uint32_t i);
heap_node_t *heap_peek(heap_t *heap);
heap_node_t heap_extract(heap_t *heap);
heap_node_t heap_insert_extract(heap_t *heap, heap_node_t elt);

int octree_to_heap_rec(octree_node_t *node, heap_t *heap);
heap_t *octree_to_heap(octree_node_t *node);
void octree_reduce(heap_t *heap, uint8_t n_colors);
void octree_palette(szim_image_t *image, octree_node_t *octree, int n_colors, unsigned char *indexed_data, unsigned char *palette, int *plte_len_res);

int dither(szim_image_t *image, unsigned char *palette, int palette_len, unsigned char *indexed_data);

void quantize_rgb(szim_image_t *image);