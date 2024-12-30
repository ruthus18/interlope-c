/* gfx.h - Graphics (Rendering Backend) */
#pragma once
#include "camera.h"
#include "platform.h"


typedef struct Shader {
    int program_id;
} Shader;


typedef struct GfxMesh {
    int vao;
    int vbo;
    int ibo;
    bool cw;  // vertex ordering (clockwise/counterwise)
} GfxMesh;


typedef struct Gfx {
    Window* window;
    bool stop_;

    struct {
        Shader* object;
    } shaders;
} Gfx;


void gfx_init();
void gfx_destroy();
Window* gfx_get_window();
bool gfx_need_stop();
void gfx_stop();

GfxMesh* gfx_load_mesh(float* vertices, int* indices, bool cw);
void gfx_unload_mesh(GfxMesh*);
void gfx_draw(Camera* camera, GfxMesh** meshes, mat4* mm_models);
