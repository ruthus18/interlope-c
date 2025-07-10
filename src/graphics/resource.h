#pragma once

#include <cglm/vec3.h>
#include "core/types.h"


typedef struct {
    // const char* name;
    u32 vao;
    u32 vbo;
    u32 ibo;

    u64 vtx_count;
    u64 ind_count;
    
    bool cw;  // vertex ordering (1 = clockwise ; 0 = counterwise)
} GfxMesh;

typedef struct {
    unsigned int id;
} GfxTexture;

typedef struct {
    u32 vao;
    u32 vbo;
    u64 vtx_count;
    vec3 color;
} GfxGeometry;

typedef struct {
    u32 vao;
    u32 vbo;
    mat4 persp_mat;
} GfxMesh2D;

typedef struct {
    u32 vao;
    u32 vbo;
    GfxTexture* texture;
} GfxSkybox;


GfxMesh* gfx_load_mesh(const char* id, f32* vtx_buf, u32* ind_buf, u64 vtx_count, u64 ind_count, bool cw);
void gfx_unload_mesh(GfxMesh*);

GfxTexture* gfx_load_texture(u8* data, u32 width, u32 height, i32 gl_format, u32 mipmap_cnt, u32 block_size);
GfxTexture* gfx_load_font_texture(u32 width, u32 height, void* data);
void gfx_unload_texture(GfxTexture*);

GfxGeometry* gfx_load_geometry(f32* lines_buf, u64 vtx_count, vec3 color);
void gfx_unload_geometry(GfxGeometry* geom);

GfxMesh2D* gfx_load_mesh_2d();
void gfx_unload_mesh_2d(GfxMesh2D* ui_data);

GfxSkybox* gfx_load_skybox(
    u8* tex_x, u8* tex_nx, u8* tex_y, u8* tex_ny, u8* tex_z, u8* tex_nz,
    u32 width, u32 height, i32 gl_format, u32 block_size
);
void gfx_unload_skybox(GfxSkybox* skybox);
