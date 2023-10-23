# sanitize-image Reference

The `options_t` struct contains all the necessary fields to tune the sanitize image operation.

```c
typedef struct
{
    input_options_t input;
    randomizer_options_t randomizer;
    resizer_options_t resizer;
    output_options_t output;
} options_t;
```

## Input options

The field `input_options_t input` is used to set limits on the input image. These parameters are **mandatory**.

```c
typedef struct
{
    image_type allowed_types[MAX_ALLOWED_TYPES];
    uint32_t max_width;
    uint32_t max_height;
    size_t max_size;
} input_options_t;
```

**`image_type allowed_types[]`** lists allowed image formats from:

- `TYPE_PNG`
- `TYPE_JPEG`

and must have its additional elements set to `NULL`.

**`uint32_t max_width`** and **`uint32_t max_height`** define the maximum allowed dimensions for the image.

**`size_t max_size`** is the maximum size (in bytes) of the image file.

## Randomizer options

The field `randomizer_options_t randomizer` sets the type of randomizer applied on the image.

```c
typedef struct
{
    randomizer_type type;
} randomizer_options_t;
```

**`randomizer_type type`** can be one of:

- `RANDOMIZER_NONE`
- `RANDOMIZER_AUTO`

The auto randomizer will randomly modify the pixels by applying a +1, 0 or -1 offset to the color value (r, g, b). This modification is invisible to the user and will result in a different binary content for the image. It is an additional defense against unknown exploits.

## Resizer options

The field `resizer_options_t resizer` specifies new dimensions for the image if needed.

```c
typedef struct
{
    resizer_type type;
    uint32_t width;
    uint32_t height;
} resizer_options_t;
```

**`resizer_type type`** can be one of:

- `RESIZER_NONE`
- `RESIZER_NN`
- `RESIZER_BILINEAR`
- `RESIZER_AUTO`

The `RESIZER_NN` is the most basic and uses the Nearest Neighbour algorithm. It works well on palette based images. The `RESIZER_BILINEAR` will interpolate between pixels and works well on most images.

Choosing `RESIZER_AUTO` will pick `RESIZER_NN` for palette based images and `RESIZER_BILINEAR` for the others.

**`uint32_t width`** and **`uint32_t height`** define the new dimensions for the image. You can set one of these to 0 and it will be computed from the image aspect ratio.

Besides standardizing the dimmensions of input images, resizing can also be seen as another randomizer for the binary content of the image.

## Output options

The field `output_options_t output` defines the output format for the image.

```c
typedef struct
{
    image_type type;
    output_png_options_t png;
    output_jpeg_options_t jpeg;
} output_options_t;
```

**`image_type type`** can be one of:

- `TYPE_INPUT`
- `TYPE_PNG`
- `TYPE_JPEG`

### Output PNG options

```c
typedef struct
{
    color_type color_type;
    int compression_level;
    enum spng_filter_choice filter;
    bool interlace;
} output_png_options_t;
```

**`color_type color_type`** can be one of:

- `COLOR_INPUT`
- `COLOR_GRAY`
- `COLOR_GRAYA`
- `COLOR_RGB`
- `COLOR_RGBA`
- `COLOR_PALETTE`

**`int compression_level`** is an integer between 0 and 9 with 9 being the maximum compression (in png compression is always lossless)

**`enum spng_filter_choice filter`**

- `SPNG_FILTER_CHOICE_ALL`

**`bool interlace`** enables line interlacing

### Output JPEG options

```c
typedef struct
{
    int quality;
    bool arith_code;
    J_DCT_METHOD dct_method;
    bool optimize;
    int smoothing;
} output_jpeg_options_t;
```

**`int quality`** is an integer between 0 and 100 with 0 being the maximum compression (in jpeg compression is lossless). A value below 90 will be noticeable in the result.

**`bool arith_code`** enables arithmetic coding

**`J_DCT_METHOD dct_method`** ?

**`bool optimize`** enables jpeg optimizations

**`int smoothing`** smoothing value
