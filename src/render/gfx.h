/* gfx.h - Graphics (Rendering Backend) */
#pragma once
#include <cglm/cglm.h>

#include "render/camera.h"

#include "core/types.h"

/* ------ GFX Data ------ */

typedef struct GfxMesh GfxMesh;
typedef struct GfxTexture GfxTexture;
typedef struct GfxGeometry GfxGeometry;

GfxMesh* gfx_mesh_load(const char* id, f32* vtx_buf, u32* ind_buf, u64 vtx_count, u64 ind_count, bool cw);
void gfx_mesh_unload(GfxMesh*);

GfxTexture* gfx_texture_load(u8* data, u32 width, u32 height, i32 gl_format);
GfxTexture* gfx_texture_load_dds(u8* data, u32 width, u32 height, i32 gl_format, u32 mipmap_cnt, u32 block_size);
void gfx_texture_unload(GfxTexture*);

GfxGeometry* gfx_geometry_load(f32* lines_buf, u64 vtx_count, vec3 color);
void gfx_geometry_unload(GfxGeometry* geom);

/* ------ High-Level Interface ------ */

void gfx_init();
void gfx_destroy();
bool gfx_need_stop();
void gfx_stop();

void gfx_update_camera(Camera* cam);  // TODO refactoring

/* ------ Renderer: Object & Object Outline ------ */

void gfx_begin_draw_objects();
void gfx_end_draw_objects();
void gfx_draw_object(GfxMesh* mesh, GfxTexture* texture, mat4 m_model, i32 id);
void gfx_set_outline_id(i32);

/* ------ Renderer: Geometry ------ */

void gfx_begin_draw_geometry();
void gfx_end_draw_geometry();
void gfx_draw_geometry(GfxGeometry* geom, vec3 pos);
