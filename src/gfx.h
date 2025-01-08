/* gfx.h - Graphics (Rendering Backend) */
#pragma once
#include <stdlib.h>
#include <cglm/cglm.h>

#include "platform/window.h"


typedef struct GfxMesh {
    const char* id;
    unsigned int vao;
    unsigned int vbo;
    unsigned int ibo;

    size_t vtx_count;
    size_t ind_count;

    bool cw;  // vertex ordering (1 = clockwise ; 0 = counterwise)
} GfxMesh;


typedef struct GfxCamera {
    mat4 m_persp;
    mat4 m_view;
} GfxCamera;


void window_init();
Window* window_get();

void gfx_init();
void gfx_destroy();
bool gfx_need_stop();
void gfx_stop();
void gfx_draw(GfxCamera* camera, GfxMesh* mesh, mat4 m_model);


GfxMesh* gfx_mesh_load(
    const char* id,
    float* vtx_buf,
    unsigned int* ind_buf,
    size_t vtx_count,
    size_t ind_count,
    bool cw
);
void gfx_mesh_unload(GfxMesh* mesh);
