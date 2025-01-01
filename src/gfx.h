/* gfx.h - Graphics (Rendering Backend) */
#pragma once
#include "platform.h"


typedef struct GfxMesh {
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
        Shader* failback;
        // Shader* object;
    } shaders;
} Gfx;


void window_init();
Window* window_get();

void gfx_init();
void gfx_destroy();
bool gfx_need_stop();
void gfx_stop();
void gfx_draw(GfxCamera* camera, mat4 m_model);
