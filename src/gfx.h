/* gfx.h - Graphics (Rendering Backend) */
#pragma once
#include "platform.h"


typedef struct GfxMesh {
    char* id;
    int vao;
    int vbo;
    int ibo;

    size_t vtx_count;
    size_t ind_count;

    bool cw;  // vertex ordering (1 = clockwise ; 0 = counterwise)
} GfxMesh;


typedef struct GfxCamera {
    mat4 m_persp;
    mat4 m_view;
} GfxCamera;


typedef struct Shader {
    int program_id;
} Shader;


typedef struct Gfx {
    Window* window;
    bool stop_;

    struct {
        Shader* objects;
        // Shader* object;
    } shaders;
} Gfx;


void window_init();
Window* window_get();

void gfx_init();
void gfx_destroy();
bool gfx_need_stop();
void gfx_stop();
void gfx_draw(GfxCamera* camera, GfxMesh* mesh, mat4 m_model);


GfxMesh* gfx_mesh_load(
    char* id, float* vtx_buf, int* ind_buf, int vtx_count, int ind_count, bool cw
);
void gfx_mesh_unload(GfxMesh* mesh);
