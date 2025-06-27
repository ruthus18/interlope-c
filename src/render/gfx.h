/* gfx.h - Graphics (Rendering Backend) */
#pragma once
#include <cglm/cglm.h>

#include "render/camera.h"

#include "core/types.h"

/* ------ GFX Data ------ */

typedef struct GfxMesh GfxMesh;
typedef struct GfxTexture GfxTexture;
typedef struct GfxGeometry GfxGeometry;
typedef struct GfxUI GfxUI;

GfxMesh* gfx_load_mesh(const char* id, f32* vtx_buf, u32* ind_buf, u64 vtx_count, u64 ind_count, bool cw);
void gfx_unload_mesh(GfxMesh*);

GfxTexture* gfx_load_texture(u8* data, u32 width, u32 height, i32 gl_format);
GfxTexture* gfx_load_dds_texture(u8* data, u32 width, u32 height, i32 gl_format, u32 mipmap_cnt, u32 block_size);
GfxTexture* gfx_load_font_texture(u32 width, u32 height, void* data);
void gfx_unload_texture(GfxTexture*);

GfxGeometry* gfx_load_geometry(f32* lines_buf, u64 vtx_count, vec3 color);
void gfx_unload_geometry(GfxGeometry* geom);

GfxUI* gfx_load_ui_data();
void gfx_unload_ui_data(GfxUI* ui_data);

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

/* ------ Renderer: UI ------ */

void gfx_begin_draw_ui();
void gfx_end_draw_ui();
void gfx_draw_ui(char* text, GfxUI* ui_data, vec2 pos, vec3 color);

/* ------ Renderer: Geometry ------ */

void gfx_begin_draw_geometry();
void gfx_end_draw_geometry();
void gfx_draw_geometry(GfxGeometry* geom, vec3 pos);
