#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "platform/file.h"
#include "gfx.h"
#include "log.h"


GfxTexture* texture_load_file(const char* texture_relpath) {
    int width = 0, height = 0, nr_channels = 0;
    const char* path = path_to_texture(texture_relpath);

    unsigned char* data = stbi_load(path, &width, &height, &nr_channels, 0);
    if (data == NULL) {
        log_error("Cannot open texture file: %s", texture_relpath);
        log_error(stbi_failure_reason());

        free((void*)path);
        exit(0);
    }

    // TODO: pass channels to decide, which mode is use (RED, RGB or RGBA)
    GfxTexture* texture = gfx_texture_load(data, width, height);
    log_success("Texture loaded: %s", texture_relpath);

    stbi_image_free(data);
    free((void*)path);
    return texture;
}
