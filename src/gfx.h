#pragma once
#include "platform.h"


typedef struct Shader {
    int program_id;
} Shader;


typedef struct GFX {
    struct {
        Shader* world_geometry;
    } shaders;
} GFX;


void gfx_init();
void gfx_destroy();
GFX* gfx_get();
void gfx_draw();
