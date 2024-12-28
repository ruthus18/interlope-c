/*
    gfx.c - Graphics backend

    Resposible for drawing stuff on screen. Incapsulating OpenGL work.
*/
#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "gfx.h"
#include "platform.h"


static GFX* self;


static
Shader* shader_create(const char* vert_p, const char* frag_p) {
    
}

static
void shader_destroy(Shader* shader) {
    free(shader);
}


void gfx_init() {
    self = malloc(sizeof(GFX));

    self->shaders.world_geometry = shader_create(
        "world_geometry.vert", "world_geometry.frag"
    );
}

void gfx_destroy() {
    shader_destroy(self->shaders.world_geometry);

    free(self);
}

GFX* gfx_get() {
    return self;
}

void gfx_draw() {
    
}
