/* gfx.h - Graphics (Rendering Backend) */
#pragma once
#include <cglm/cglm.h>

#include "platform/window.h"
#include "types.h"


typedef struct GfxGeometry {
    u32 vao;
    u32 vbo;

    u64 vtx_count;
} GfxGeometry;


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


typedef struct GfxObject {
    GfxMesh* mesh;
    GfxTexture* texture;
    mat4 m_model;
} GfxObject;


typedef struct GfxScene {
    GfxCamera* camera;
    GfxObject objects[1024];  // FIXME
    u64 objects_count;
} GfxScene;


void window_init();
Window* window_get();

/* ------ Gfx main interface ------ */
void gfx_init();
void gfx_destroy();
bool gfx_need_stop();
void gfx_stop();

void gfx_begin_draw();
void gfx_end_draw();
void gfx_draw(GfxScene*);

/* ------ Mesh ------ */
GfxMesh* gfx_mesh_load(
    const char* id,
    f32* vtx_buf,
    u32* ind_buf,
    u64 vtx_count,
    u64 ind_count,
    bool cw
);
void gfx_mesh_unload(GfxMesh*);

/* ------ Texture ------ */
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

/* ------ Geometry ------ */
GfxGeometry* gfx_geometry_load(f32* vtx_buf, u64 vtx_count);
void gfx_geometry_unload(GfxGeometry* geom);
