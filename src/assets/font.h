#pragma once
#include "render/gfx.h"

typedef struct Glyph {
    u8 code;                // Character code
    GfxTexture* texture;    // Glyph texture
    ivec2 size;             // Glyph size
    ivec2 bearing;          // Offset from baseline to left/top of glyph
    u32 advance;            // Offset to advance to next glyph
} Glyph;

typedef struct Font {
    Glyph chars[128];
} Font;


void font_load_default();
void font_unload_default();
Font* font_get_default();
