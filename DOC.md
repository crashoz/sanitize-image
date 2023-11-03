# sanitize-image Reference

The `szim_options_t` struct contains all the necessary fields to tune the sanitize image operation.

```c
typedef struct
{
    szim_input_options_t input;
    szim_randomizer_options_t randomizer;
    szim_resizer_options_t resizer;
    szim_output_options_t output;
} szim_options_t;
```

## Input options

The field `szim_input_options_t input` is used to set limits on the input image. These parameters are **mandatory**.

```c
typedef struct
{
    szim_image_type allowed_types[MAX_ALLOWED_TYPES];
    uint32_t max_width;
    uint32_t max_height;
    size_t max_size;
} szim_input_options_t;
```

**`szim_image_type allowed_types[]`** lists allowed image formats from:

- `SZIM_TYPE_PNG`
- `SZIM_TYPE_JPEG`

and must have its additional elements set to `NULL`.

**`uint32_t max_width`** and **`uint32_t max_height`** define the maximum allowed dimensions for the image.

**`size_t max_size`** is the maximum size (in bytes) of the image file.

## Randomizer options

The field `szim_randomizer_options_t randomizer` sets the type of randomizer applied on the image.

```c
typedef struct
{
    szim_randomizer_type type;
} szim_randomizer_options_t;
```

**`szim_randomizer_type type`** can be one of:

- `SZIM_RANDOMIZER_NONE`
- `SZIM_RANDOMIZER_AUTO`

The auto randomizer will randomly modify the pixels by applying a +1, 0 or -1 offset to the color value (r, g, b). This modification is invisible to the user and will result in a different binary content for the image. It is an additional defense against unknown exploits.

## Resizer options

The field `szim_resizer_options_t resizer` specifies new dimensions for the image if needed.

```c
typedef struct
{
    szim_resizer_type type;
    uint32_t width;
    uint32_t height;
} szim_resizer_options_t;
```

**`resizer_type type`** can be one of:

- `SZIM_RESIZER_NONE`
- `SZIM_RESIZER_NN`
- `SZIM_RESIZER_BILINEAR`
- `SZIM_RESIZER_AUTO`

The `SZIM_RESIZER_NN` is the most basic and uses the Nearest Neighbour algorithm. It works well on palette based images. The `SZIM_RESIZER_BILINEAR` will interpolate between pixels and works well on most images.

Choosing `SZIM_RESIZER_AUTO` will pick `SZIM_RESIZER_NN` for palette based images and `SZIM_RESIZER_BILINEAR` for the others.

**`uint32_t width`** and **`uint32_t height`** define the new dimensions for the image. You can set one of these to 0 and it will be computed from the image aspect ratio.

Besides standardizing the dimmensions of input images, resizing can also be seen as another randomizer for the binary content of the image.

## Output options

The field `szim_output_options_t output` defines the output format for the image.

```c
typedef struct
{
    szim_image_type type;
    szim_output_png_options_t png;
    szim_output_jpeg_options_t jpeg;
} szim_output_options_t;
```

**`szim_image_type type`** can be one of:

- `SZIM_TYPE_INPUT`
- `SZIM_TYPE_PNG`
- `SZIM_TYPE_JPEG`

### Output PNG options

```c
typedef struct
{
    szim_color_type color_type;
    int compression_level;
    enum spng_filter_choice filter;
    bool interlace;
} szim_output_png_options_t;
```

**`color_type color_type`** can be one of:

- `SZIM_COLOR_INPUT`
- `SZIM_COLOR_GRAY`
- `SZIM_COLOR_GRAYA`
- `SZIM_COLOR_RGB`
- `SZIM_COLOR_RGBA`
- `SZIM_COLOR_PALETTE`

**`int compression_level`** is an integer between 0 and 9 with 9 being the maximum compression (in png compression is always lossless).

**`enum spng_filter_choice filter`** can be one or many of:

- `SPNG_DISABLE_FILTERING` = 0,
- `SPNG_FILTER_CHOICE_NONE` = 8,
- `SPNG_FILTER_CHOICE_SUB` = 16,
- `SPNG_FILTER_CHOICE_UP` = 32,
- `SPNG_FILTER_CHOICE_AVG` = 64,
- `SPNG_FILTER_CHOICE_PAETH` = 128,
- `SPNG_FILTER_CHOICE_ALL` = (8|16|32|64|128)

**`bool interlace`** enables a degraded version of the image to load first.

### Output JPEG options

```c
typedef struct
{
    int quality;
    bool arith_code;
    J_DCT_METHOD dct_method;
    bool optimize;
    int smoothing;
    bool progressive;
} szim_output_jpeg_options_t;
```

**`int quality`** is an integer between 0 and 100 with 0 being the maximum compression (in jpeg compression is lossless). A value below 90 will be noticeable in the result.

**`bool arith_code`** enables arithmetic coding (instead of Huffman)

**`J_DCT_METHOD dct_method`** can be one of:

- `JDCT_ISLOW`: accurate integer method
- `JDCT_IFAST`: less accurate integer method [legacy feature]
- `JDCT_FLOAT`: floating-point method [legacy feature]
- `JDCT_DEFAULT`: default method (normally JDCT_ISLOW)
- `JDCT_FASTEST`: fastest method (normally JDCT_IFAST)

**`bool optimize`** enables jpeg optimizations: compute optimal Huffman coding tables for the image (default false)

**`int smoothing`** smoothing value between 0 and 100 with 100 being maximum smoothing (default 0)

**`bool progressive`** enables jpeg progressive decoding (similar to png's interlace)
