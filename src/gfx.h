/* gfx.h - Graphics (Rendering Backend) */
#pragma once
#include <stdlib.h>
#include <cglm/cglm.h>

#include "platform/window.h"
#include "types.h"


typedef struct GfxMesh {
    const char* name;
    u32 vao;
    u32 vbo;
    u32 ibo;

    u64 vtx_count;
    u64 ind_count;

    bool cw;  // vertex ordering (1 = clockwise ; 0 = counterwise)
} GfxMesh;


typedef struct GfxTexture {
    unsigned int id;
} GfxTexture;


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
void gfx_begin_draw();
void gfx_end_draw();
void gfx_draw(GfxCamera* camera, GfxMesh* mesh, GfxTexture* texture, mat4 m_model);

GfxMesh* gfx_mesh_load(
    const char* id,
    f32* vtx_buf,
    u32* ind_buf,
    u64 vtx_count,
    u64 ind_count,
    bool cw
);
void gfx_mesh_unload(GfxMesh*);

GfxTexture* gfx_texture_load(
    u8* data,
    u32 width,
    u32 height,
    i32 gl_format
);
GfxTexture* gfx_texture_load_dds(
    u8* data,
    u32 width,
    u32 height,
    i32 gl_format,
    u32 mipmap_cnt,
    u32 block_size
);
void gfx_texture_unload(GfxTexture*);
