#include <stdbool.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <cglm/ivec2.h>

#include "font.h"

#include "graphics/gfx.h"
#include "core/log.h"
#include "core/types.h"


#define DEFAULT_FONT_PATH "assets/fonts/berkeley_mono_bold.ttf"


Font default_font;

void font_load_default() {
    bool err;

    FT_Library ft;
    err = FT_Init_FreeType(&ft);
    if (err)  log_exit("ERROR::FREETYPE: Could not init FreeType Library");

    FT_Face face;
    err = FT_New_Face(ft, DEFAULT_FONT_PATH, 0, &face);
    if (err)  log_exit("ERROR::FREETYPE: Failed to load default font");

    // set width and height (0 width -> dynamically calculated)
    FT_Set_Pixel_Sizes(face, 0, 48);

    // Load first 128 ASCII characters
    for (u8 c = 0; c < 128; c++) {
        // FT_LOAD_RENDER - create an 8-bit grayscale bitmap image
        err = FT_Load_Char(face, c, FT_LOAD_RENDER);
        if (err) {
            log_error("ERROR::FREETYPE: Failed to load Glyph for character %d", c);
            continue;
        }

        GfxTexture* font_texture = gfx_load_font_texture(
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            face->glyph->bitmap.buffer
        );
        
        default_font.chars[c].texture = font_texture;
        default_font.chars[c].code = c;
        default_font.chars[c].size[0] = face->glyph->bitmap.width;
        default_font.chars[c].size[1] = face->glyph->bitmap.rows;
        default_font.chars[c].bearing[0] = face->glyph->bitmap_left;
        default_font.chars[c].bearing[1] = face->glyph->bitmap_top;
        default_font.chars[c].advance = face->glyph->advance.x;
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}


void font_unload_default() {
    for (int i = 0; i < 128; i++) {
        if (default_font.chars[i].texture) {
            gfx_unload_texture(default_font.chars[i].texture);
        }
    }
}


Font* font_get_default() {
    return &default_font;
}
