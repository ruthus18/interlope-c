/* gfx.h - Graphics (Rendering Backend) */
#pragma once
#include <cglm/cglm.h>

#include "types.h"


/* ------ GFX Data ------ */

typedef struct GfxMesh GfxMesh;
typedef struct GfxTexture GfxTexture;
typedef struct GfxGeometry GfxGeometry;

GfxMesh* gfx_mesh_load(const char* id, f32* vtx_buf, u32* ind_buf, u64 vtx_count, u64 ind_count, bool cw);
void gfx_mesh_unload(GfxMesh*);

GfxTexture* gfx_texture_load(u8* data, u32 width, u32 height, i32 gl_format);
GfxTexture* gfx_texture_load_dds(u8* data, u32 width, u32 height, i32 gl_format, u32 mipmap_cnt, u32 block_size);
void gfx_texture_unload(GfxTexture*);

GfxGeometry* gfx_geometry_load(f32* vtx_buf, u64 vtx_count);
void gfx_geometry_unload(GfxGeometry* geom);

/* ------ GFX main interface ------ */

void gfx_init();
void gfx_destroy();
bool gfx_need_stop();
void gfx_stop();

void gfx_update_camera(mat4 m_persp, mat4 m_view);

void gfx_begin_draw_objects();
void gfx_end_draw_objects();
void gfx_draw_object(GfxMesh* mesh, GfxTexture* texture, mat4 m_model);
