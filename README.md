# sanitize-image

Fast image sanitizer to securely store potentially malicious image input.

## Features

By default, the sanitizer will decode a binary buffer as an image, or fail gracefuly. A randomizer is then applied to shuffle the binary content, breaking unexpected exploits. The output size can be fixed with the resizer. Finally the output type is specified with the encoder.

The intended use is to process user-uploaded images and store them on your filesystem, so that they can be served safely from a static file server.

### Supported formats

- Image types: **png**, **jpeg**
- Color types: grayscale, grayscale alpha, rgb, rgba, palette
- Bit depth: 8

## Install

This library needs a C compiler and uses the CMake build system. It will by default download its dependencies from github.

Get the latest release from github, make a `build` directory inside the source tree and run cmake to prepare the build.

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
```

Then build the library

```
cmake --build .
```

### Testing

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
cd tests
./test_all
```

## Getting started

The input is an image binary buffer as `unsigned char*`

All settings are specified in the `szim_options_t` struct. You can get good default settings with:

```c
szim_options_t options = szim_default_options()
```

This will output an image in the same type and size as the input.

To process the image use the `szim_sanitize` function:

```c
int szim_sanitize(unsigned char *data, size_t size, image_type type, const char *path, szim_options_t options, char *res_path, size_t res_path_len);
```

With the following parameters:

- `unsigned char *data` = the input image
- `size_t size` = length (in bytes) of data
- `image_type type` = the data image type or `TYPE_UNKNOWN` to guess it
- `const char *path` = the output file path **without extension**
- `szim_options_t options` = the options struct
- `char *res_path` = (output) will contain the full output path
- `size_t res_path_len` = length of the `res_path` buffer

Full minimal example:

```c
#define BUFFER_SIZE 4096 * 16
#define MAX_PATH 4096
unsigned char *buffer = malloc(BUFFER_SIZE);
FILE *f = fopen("./in_image.png", "rb");
int n = fread(buffer, sizeof(unsigned char), BUFFER_SIZE, f);
fclose(f);

szim_options_t options = szim_default_options();
char res_path[MAX_PATH];
int ret = szim_sanitize(buffer, n, SZIM_TYPE_UNKNOWN, "./out_image", options, res_path, MAX_PATH);

printf("image has been saved as: %s\n", res_path);

free(buffer);
```

## Documentation

See [DOC.md](DOC.md)

## References about untrusted files

- https://libspng.org/docs/decode/#decoding-untrusted-files
- https://cheatsheetseries.owasp.org/cheatsheets/File_Upload_Cheat_Sheet.html
- https://cheatsheetseries.owasp.org/cheatsheets/Input_Validation_Cheat_Sheet.html#file-upload-validation
- https://owasp.org/www-community/vulnerabilities/Unrestricted_File_Upload
- https://security.stackexchange.com/questions/8587/how-can-i-be-protected-from-pictures-vulnerabilities/8625#8625
- https://learn.microsoft.com/en-us/windows/win32/fileio/naming-a-file?redirectedfrom=MSDN#naming-conventions
